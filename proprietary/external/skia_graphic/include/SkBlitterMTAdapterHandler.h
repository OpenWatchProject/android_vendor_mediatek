/*
 * Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Created on: 2017-11-30
 * Author: Christ Sun
 */

#ifndef __SKBlITTERMTADATER_HANDLER_H__
#define __SKBlITTERMTADATER_HANDLER_H__
#include "SkBlitterAdapterHandler.h"


/*mtk multi-threads optimization*/
class BlitDirectlyNoSPHandlerMT: public BlitHandler {
public:
    void handleRequest(BlitRectHandlerParam *handlerParam,
                        const int &x, int &y,
                        const int &width, int &height) override;
};


class BlitSpecial2DHandlerMT: public BlitHandler {
public:
    void handleRequest(BlitRectHandlerParam *handlerParam,
                        const int &x, int &y,
                        const int &width, int &height) override;
};

/*blit indirectly without xfer with mt*/
class BlitIndirectlyNoXferHandlerMT: public BlitHandler {
public:
    void handleRequest(BlitRectHandlerParam *handlerParam,
                        const int &x, int &y,
                        const int &width, int &height) override;
};

#endif
