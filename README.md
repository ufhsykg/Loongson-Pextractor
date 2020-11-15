# Loongson-Pextractor
│  .gitattributes
│  .gitignore
│  LICENSE
│  README.md
│  Readme.txt
│  
├─Loongson drive
│  ├─bin
│  │      junbian.ko
│  │      JUNNEW
│  │      
│  └─src
│          junbian.c
│          junbian.code-workspace
│          junbian.ko
│          junbian.mod.c
│          junbian.mod.o
│          junbian.o
│          JUNNEW
│          Makefile
│          Module.symvers
│          modules.order
│          test_tr-mem.c
│          
├─Psplash_loong
│  │  aclocal.m4
│  │  AUTHORS
│  │  autogen.sh
│  │  ChangeLog
│  │  compile
│  │  config.h
│  │  config.h.in
│  │  config.log
│  │  config.status
│  │  configure
│  │  configure.ac
│  │  COPYING
│  │  depcomp
│  │  INSTALL
│  │  install-sh
│  │  JunBian-img.h
│  │  make-image-header.sh
│  │  Makefile
│  │  Makefile.am
│  │  Makefile.in
│  │  missing
│  │  NEWS
│  │  psplash
│  │  psplash-bar-img.h
│  │  psplash-colors.h
│  │  psplash-config.h
│  │  psplash-console.c
│  │  psplash-console.h
│  │  psplash-console.o
│  │  psplash-fb.c
│  │  psplash-fb.h
│  │  psplash-fb.o
│  │  psplash-hand-img.h
│  │  psplash-poky-img.h
│  │  psplash-systemd.c
│  │  psplash-write
│  │  psplash-write.c
│  │  psplash-write.o
│  │  psplash.c
│  │  psplash.doap
│  │  psplash.h
│  │  psplash.o
│  │  radeon-font.h
│  │  README
│  │  stamp-h1
│  │  
│  ├─autom4te.cache
│  │      output.0
│  │      output.1
│  │      requests
│  │      traces.0
│  │      traces.1
│  │      
│  ├─base-images
│  │      poky-logo.png
│  │      poky-logo2.png
│  │      psplash-bar.png
│  │      psplash-hand.png
│  │      psplash-poky.png
│  │      
│  └─loongson-images
│          JunBian.png
│          
├─QT application
│  ├─Client(for Loongson Pi 2)
│  │  ├─bin
│  │  │  │  client
│  │  │  │  
│  │  │  └─images
│  │  │          addFace_ok.jpg
│  │  │          Background.png
│  │  │          copyright-0.png
│  │  │          face.png
│  │  │          junbian.jpg
│  │  │          rec_ok.jpg
│  │  │          slab_BG.png
│  │  │          SYS_BG.png
│  │  │          
│  │  ├─Link
│  │  │      clisocket.o
│  │  │      main.o
│  │  │      mainwindow.o
│  │  │      Makefile
│  │  │      moc_mainwindow.cpp
│  │  │      moc_mainwindow.o
│  │  │      moc_predefs.h
│  │  │      opencv_deal.o
│  │  │      v4l2_cap.o
│  │  │      
│  │  └─src
│  │          client.pro
│  │          client.pro.user
│  │          client.pro.user.99d7fd8
│  │          clisocket.c
│  │          clisocket.h
│  │          main.cpp
│  │          main.h
│  │          mainwindow.cpp
│  │          mainwindow.h
│  │          moc_mainwindow.cpp
│  │          opencv_deal.cpp
│  │          opencv_deal.h
│  │          v4l2_cap.c
│  │          v4l2_cap.h
│  │          
│  └─Server(for Ubuntu 18.04)
│      ├─bin
│      │  │  faces.csv
│      │  │  haarcascade_frontalface_alt.xml
│      │  │  server
│      │  │  update_csv
│      │  │  
│      │  ├─faces
│      │  │  └─1_loongson
│      │  │          1.jpg
│      │  │          10.jpg
│      │  │          2.jpg
│      │  │          3.jpg
│      │  │          4.jpg
│      │  │          5.jpg
│      │  │          6.jpg
│      │  │          7.jpg
│      │  │          8.jpg
│      │  │          9.jpg
│      │  │          
│      │  └─images
│      │          addFace_ok.jpg
│      │          Background.png
│      │          copyright-0.png
│      │          face.png
│      │          junbian.jpg
│      │          rec_ok.jpg
│      │          slab_BG.png
│      │          SYS_BG.png
│      │          
│      ├─Link
│      │      main.o
│      │      mainwindow.o
│      │      Makefile
│      │      moc_mainwindow.cpp
│      │      moc_mainwindow.o
│      │      moc_predefs.h
│      │      opencv_deal.o
│      │      svrsocket.o
│      │      usermngr.o
│      │      
│      └─src
│              main.cpp
│              main.h
│              mainwindow.cpp
│              mainwindow.h
│              moc_mainwindow.cpp
│              opencv_deal.cpp
│              opencv_deal.h
│              server.pro
│              server.pro.user
│              svrsocket.c
│              svrsocket.h
│              usermngr.cpp
│              usermngr.h
│              
├─Solid design
│  ├─Cutdev
│  │      基座上层.SLDPRT
│  │      电磁铁装置.SLDPRT
│  │      自动切纸机.SLDASM
│  │      
│  └─Drawdev
│          同步轮 20齿 带宽10、(1).SLDPRT
│          棍子(2).SLDPRT
│          棍子固定(2).SLDPRT
│          电动机.SLDPRT
│          电动机固定架.SLDPRT
│          送纸装置(1).SLDASM
│          
├─Startup scripts
│      rc.local
│      
└─UI design
        comBG.psd
        StartUp.psd
        sysbg.psd
        
