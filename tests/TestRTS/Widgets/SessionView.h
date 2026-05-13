#pragma once

#include "PanelWidget.h"

class SessionInstance;

class SessionView : public PanelWidget
{
public:
    
    SessionView(SessionInstance* sessionInstance);
};
