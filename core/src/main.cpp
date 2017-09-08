#include <Application.hpp>
#include <memory>
#include "MyAppListener.hpp"

int main(int argc, char* argv[])
{    
    /*std::shared_ptr<gel::DesktopConfiguration> desktop = std::make_shared<gel::DesktopConfiguration>();
    std::shared_ptr<gel::MobileConfiguration> mobile = std::make_shared<gel::MobileConfiguration>();
    desktop->fullscreen = true;
    desktop->setBetterAntiAlias();
    mobile->setBetterAntiAlias();*/
    
    std::unique_ptr<gel::Application> app = std::make_unique<gel::Application>(
        /*desktop,mobile,*/std::make_shared<MyAppListener>());
    return 0;
}
