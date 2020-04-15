# Architecture of the container #

Components
============

The container contains the following components :
- An Ubuntu base system
- The tini + supervisord startup and daemon control system
- Nginx Web server
- A backend ("novnc2") Python Web app providing an API (written with
  Flask) on port 6079
- A frontend VueJS Web app displayed to the user, which will wrap noVNC
- noVNC + WebSockify providing the Web VNC client in an HTML5 canvas
- Xvfb running the X11 server in memory
- x11vnc exporting the X11 display through VNC
- and all regular X applications, like the LXDE desktop and apps

Wiring them all
------------------

Internally, Xvfb will be started in DISPLAY :1, then x11vnc will
provide access to it on the default VNC port (5900).

noVNC will be started listening to HTTP requests on port 6081.
It is possible to connect directly to port 6081 of the container, to
only use the regular noVNC Web interface (provided it is exported by
the container).

Above noVNC stands the VueJS frontend Web app provided by nginx, which
will proxy the noVNC canvas, and will add some useful features over
noVNC.

User-oriented features
==========================

The Web frontend adds the following features :
- upon display of the Web page, the app will detect the size of the
  Web browser's window, and will invoke the backend API so as to make
  sure the noVNC rendering ajusts to that size
- provide a flash video rendering transporting the sound (???)

