/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "exr_export.h"

#include <QCheckBox>
#include <QSlider>
#include <QApplication>

#include <KoDialog.h>
#include <kpluginfactory.h>
#include <QFileInfo>

#include <KoColorSpaceConstants.h>
#include <KisImportExportManager.h>

#include <kis_properties_configuration.h>
#include <kis_config.h>
#include <KisDocument.h>
#include <kis_image.h>
#include <kis_group_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>

#include "exr_converter.h"


class KisExternalLayer;

K_PLUGIN_FACTORY_WITH_JSON(ExportFactory, "krita_exr_export.json", registerPlugin<exrExport>();)

exrExport::exrExport(QObject *parent, const QVariantList &) : KisImportExportFilter(parent)
{
}

exrExport::~exrExport()
{
}

KisPropertiesConfigurationSP exrExport::defaultConfiguration(const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("flatten", false);
    return cfg;
}

KisPropertiesConfigurationSP exrExport::lastSavedConfiguration(const QByteArray &from, const QByteArray &to) const
{
    KisPropertiesConfigurationSP cfg = defaultConfiguration(from, to);
    QString filterConfig = KisConfig().exportConfiguration("EXR");
    cfg->fromXML(filterConfig, false);
    return cfg;
}

KisConfigWidget *exrExport::createConfigurationWidget(QWidget *parent, const QByteArray &/*from*/, const QByteArray &/*to*/) const
{
    return new KisWdgOptionsExr(parent);
}

KisImportExportFilter::ConversionStatus exrExport::convert(KisDocument *document, QIODevice */*io*/,  KisPropertiesConfigurationSP configuration)
{
    KisImageWSP image = document->image();

    KoDialog kdb;
    kdb.setWindowTitle(i18n("OpenEXR Export Options"));
    kdb.setButtons(KoDialog::Ok | KoDialog::Cancel);
    KisConfigWidget *wdg = createConfigurationWidget(&kdb, KisDocument::nativeFormatMimeType(), "image/x-exr");
    kdb.setMainWidget(wdg);
    kdb.resize(kdb.minimumSize());

    // If a configuration object was passed to the convert method, we use that, otherwise we load from the settings
    KisPropertiesConfigurationSP cfg(new KisPropertiesConfiguration());
    if (configuration) {
        cfg->fromXML(configuration->toXML());
    }
    else {
        cfg = lastSavedConfiguration(KisDocument::nativeFormatMimeType(), "image/x-exr");
    }
    wdg->setConfiguration(cfg);

    if (!batchMode() ) {
        QApplication::restoreOverrideCursor();
        if (kdb.exec() == QDialog::Rejected) {
            return KisImportExportFilter::UserCancelled;
        }
        cfg = wdg->configuration();
        KisConfig().setExportConfiguration("EXR", *cfg.data());
    }

    exrConverter kpc(document, !batchMode());

    KisImageBuilder_Result res;

    if (cfg->getBool("flatten")) {
        // the image must be locked at the higher levels
        KIS_SAFE_ASSERT_RECOVER_NOOP(document->image()->locked());

        KisPaintDeviceSP pd = new KisPaintDevice(*image->projection());
        KisPaintLayerSP l = new KisPaintLayer(image, "projection", OPACITY_OPAQUE_U8, pd);

        res = kpc.buildFile(filename(), l);
    }
    else {
        // the image must be locked at the higher levels
        KIS_SAFE_ASSERT_RECOVER_NOOP(document->image()->locked());
        res = kpc.buildFile(filename(), image->rootLayer());
    }

    dbgFile << " Result =" << res;
    switch (res) {
    case KisImageBuilder_RESULT_INVALID_ARG:
        document->setErrorMessage(i18n("This layer cannot be saved to EXR."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_EMPTY:
        document->setErrorMessage(i18n("The layer does not have an image associated with it."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_NO_URI:
        document->setErrorMessage(i18n("The filename is empty."));
        return KisImportExportFilter::CreationError;

    case KisImageBuilder_RESULT_NOT_LOCAL:
        document->setErrorMessage(i18n("EXR images cannot be saved remotely."));
        return KisImportExportFilter::InternalError;

    case KisImageBuilder_RESULT_UNSUPPORTED_COLORSPACE:
        document->setErrorMessage(i18n("Colorspace not supported: EXR images must be 16 or 32 bits floating point RGB."));
        return KisImportExportFilter::WrongFormat;

    case KisImageBuilder_RESULT_OK:
        return KisImportExportFilter::OK;
    default:
        break;
    }

    document->setErrorMessage(i18n("Internal Error"));
    return KisImportExportFilter::InternalError;

}


void KisWdgOptionsExr::setConfiguration(const KisPropertiesConfigurationSP cfg)
{
    chkFlatten->setChecked(cfg->getBool("flatten", false));
}

KisPropertiesConfigurationSP KisWdgOptionsExr::configuration() const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    cfg->setProperty("flatten", chkFlatten->isChecked());
    return cfg;
}


#include <exr_export.moc>

