#pragma once

#include "Widget.h"

#include "../Controls/Console.h"

class ConsoleView : public Widget
{
public:
    
    void Initialize() override;

    void Render() override;
    
private:

    Console Console;
};
