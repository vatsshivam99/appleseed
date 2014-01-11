
//
// This source file is part of appleseed.
// Visit http://appleseedhq.net/ for additional information and resources.
//
// This software is released under the MIT license.
//
// Copyright (c) 2010-2013 Francois Beaune, Jupiter Jazz Limited
// Copyright (c) 2014 Francois Beaune, The appleseedhq Organization
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#ifndef APPLESEED_STUDIO_MAINWINDOW_STATUSBAR_H
#define APPLESEED_STUDIO_MAINWINDOW_STATUSBAR_H

// appleseed.studio headers.
#include "mainwindow/rendering/renderingtimer.h"

// Qt headers.
#include <QLabel>
#include <QObject>

// Standard headers.
#include <string>

// Forward declarations.
class QTimerEvent;

namespace appleseed {
namespace studio {

class StatusBar
  : public QLabel
{
    Q_OBJECT

  public:
    StatusBar();

    void set_text(const std::string& text);

    void start_rendering_time_display(RenderingTimer* rendering_timer);

    void stop_rendering_time_display();

  private:
    RenderingTimer* m_rendering_timer;
    int             m_timer_id;

    virtual void timerEvent(QTimerEvent* event);
};

}       // namespace studio
}       // namespace appleseed

#endif  // !APPLESEED_STUDIO_MAINWINDOW_STATUSBAR_H
