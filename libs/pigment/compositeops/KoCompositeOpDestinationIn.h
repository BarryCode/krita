/*
 *  Copyright (c) 2016 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *  Copyright (c) 2012 José Luis Vergara <pentalis@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KOCOMPOSITEOPDESTINATIONIN_H_
#define _KOCOMPOSITEOPDESTINATIONIN_H_

#include "KoCompositeOpBase.h"

/**
 *  Generic implementation of the Destination-in composite op, based off the behind composite op.
 *  This is necessary for Open Raster support.
 *  https://www.w3.org/TR/compositing-1/
 */
template<class CS_Traits>
class KoCompositeOpDestinationIn : public KoCompositeOpBase<CS_Traits, KoCompositeOpDestinationIn<CS_Traits> >
{
    typedef KoCompositeOpBase<CS_Traits, KoCompositeOpDestinationIn<CS_Traits> > base_class;
    typedef typename CS_Traits::channels_type channels_type;

    static const qint8 channels_nb = CS_Traits::channels_nb;
    static const qint8 alpha_pos   = CS_Traits::alpha_pos;

public:
    KoCompositeOpDestinationIn(const KoColorSpace * cs)
        : base_class(cs, COMPOSITE_DESTINATION_IN, i18n("Destination In"), KoCompositeOp::categoryMix()) { }

public:
    template<bool alphaLocked, bool allChannelFlags>
    inline static channels_type composeColorChannels(const channels_type* src, channels_type srcAlpha,
                                                     channels_type*       dst, channels_type dstAlpha,
                                                     channels_type  maskAlpha, channels_type  opacity,
                                                     const QBitArray& channelFlags                    )  {
        using namespace Arithmetic;
        Q_UNUSED(src);
        Q_UNUSED(dst);
        Q_UNUSED(channelFlags);

        channels_type appliedAlpha       = mul(maskAlpha, srcAlpha, opacity);

        channels_type newDstAlpha        = mul(dstAlpha, appliedAlpha);

        return newDstAlpha;
    }
};

#endif  // _KOCOMPOSITEOPDESTINATIONIN_H_
