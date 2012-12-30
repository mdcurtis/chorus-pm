option( FRONTEND_INTERACTIVE "Add interactive control features to frontend (needs ncurses and lirc)" OFF )
option( FRONTEND_DAEMONIZE "Add support for daemonizing (backgrounding) the frontend" OFF )
option( FRONTEND_SIGNALS "Use signals for restart (only on Unix platforms)" ON )

if( UNIX AND FRONTEND_SIGNALS )
  add_definitions( -DUSE_SIGNALS_FOR_RESTART )
endif()

if( FRONTEND_INTERACTIVE)
  add_definitions( -DINTERACTIVE )
  pkg_check_extended( NCURSES ncurses ncurses.h ncurses REQUIRED )
  pkg_check_extended( LIRC lirc lirc/lirc_client.h lirc_client REQUIRED )

  include_directories( ${NCURSES_INCLUDE_DIRS} ${LIRC_INCLUDE_DIRS} )
endif() 

if( FRONTEND_DAEMONIZE )
  add_definitions( -DDAEMONIZE )
endif()

add_executable( squeezeslave 
  squeezeslave.c 
  help.c 
  daemonize.c 
  interactive.c )

target_link_libraries( squeezeslave slimproto )

if( FRONTEND_INTERACTIVE )
  target_link_libraries( squeezeslave ${NCURSES_LIBRARIES} ${LIRC_LIBRARIES} )
endif()
