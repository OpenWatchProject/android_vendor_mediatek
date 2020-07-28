/*
 * Copyright (c) 2013-2017 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Tui.h"
#include "TuiManager.h"

namespace vendor {
namespace trustonic {
namespace tee {
namespace tui {
namespace V1_0 {
namespace implementation {

struct Tui::Impl {
    TuiManager tui_manager;
    std::shared_ptr<TrustonicService> parent;

};

// Using new here because std::make_unique is not part of C++11
Tui::Tui(std::shared_ptr<TrustonicService> service_): pimpl_(new Impl) {
    pimpl_->parent = service_;
}

Tui::~Tui() {
}

// Methods from ::vendor::trustonic::tee::tui::V1_0::ITui follow.
Return<uint32_t> Tui::notifyReeEvent(uint32_t code) {
    return pimpl_->tui_manager.notifyReeEvent(code);
}

Return<void> Tui::registerTuiCallback(const sp<::vendor::trustonic::tee::tui::V1_0::ITuiCallback>& callback) {
    pimpl_->tui_manager.registerTuiCallback(callback);
    /* I consider that registerTeeCallback should be called on ITee prior to this call
     * so the tee_callback will be available at that time
     */
    pimpl_->tui_manager.registerTeeCallback(pimpl_->parent->getTeeCallback());
    return Void();
}

Return<uint32_t> Tui::notifyScreenSizeUpdate(uint32_t width, uint32_t height) {
    return pimpl_->tui_manager.notifyScreenSizeUpdate(width, height);
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace tui
}  // namespace tee
}  // namespace trustonic
}  // namespace vendor
