#include "ConsoleView.h"

#include "../App/App.h"

void ConsoleView::Initialize()
{
    Widget::Initialize();

    Console.Init(App::Get().GetLogger());
}

void ConsoleView::Render()
{
    Widget::Render();

    Console.Draw();
}
