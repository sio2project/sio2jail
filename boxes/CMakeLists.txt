ADD_CUSTOM_COMMAND(
    OUTPUT
        minimal.tar.gz busybox.tar.gz python2.tar.gz python3.tar.gz
    COMMAND
        ${CMAKE_SOURCE_DIR}/boxes/make_boxes.sh
    DEPENDS
        make_boxes.sh
    )

ADD_CUSTOM_COMMAND(
    OUTPUT
        boxes_extraction_stamp.txt
    COMMAND
        tar -xvf minimal.tar.gz
            && tar -xvf busybox.tar.gz
            && tar -xvf python2.tar.gz
            && tar -xvf python3.tar.gz
            && touch boxes_extraction_stamp.txt
    DEPENDS
        minimal.tar.gz busybox.tar.gz python2.tar.gz python3.tar.gz
    )

ADD_CUSTOM_TARGET(boxes
    DEPENDS
        boxes_extraction_stamp.txt
    )

INSTALL(FILES download_boxes.sh make_boxes.sh utils.sh
    DESTINATION ${INSTALLATION_DIR}/boxes
    PERMISSIONS  OWNER_READ OWNER_EXECUTE)
