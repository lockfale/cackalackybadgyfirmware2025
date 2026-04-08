#pragma once
/*
    badgeMenu.h -- part of the cackalackabadgy2023 project.
    Function declarations for interacting with the badge menu
*/
namespace BMenu {

    void updateMenu( );
    void enableMenu( );
    void disableMenu( );
    auto isMenuRunning( ) -> bool;
    void moveMenuDown( );
    void moveMenuUp( );
    void enterMenu( );
    void escMenu( );

}
