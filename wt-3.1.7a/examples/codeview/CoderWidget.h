// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CODER_WIDGET_H_
#define CODER_WIDGET_H_

#include <Wt/WContainerWidget>

#include "CodeSession.h"

class BufferEditorWidget;

class CoderWidget : public Wt::WContainerWidget
{
public:
  CoderWidget();
  virtual ~CoderWidget();

private:
  Wt::WApplication *app_;
  CodeSession *session_;
  CodeSession::Connection connection_;

  Wt::WContainerWidget *buffers_;
  Wt::WText *observerCount_;

  void addBuffer();
  void insertBuffer(int index);
  void changed(BufferEditorWidget *editor);
  void sessionChanged();
};

#endif // CODER_WIDGET_H_
