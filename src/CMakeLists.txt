include_directories(${GPGME_INCLUDES})

########### next target ###############

set(basketcommon_LIB_SRCS
    aboutdata.cpp
    archive.cpp
    backgroundmanager.cpp
    backup.cpp
    basketfactory.cpp
    basketlistview.cpp
    basketproperties.cpp
    basketscene.cpp
    basketstatusbar.cpp
    basketview.cpp
    bnpview.cpp
    colorpicker.cpp
    crashhandler.cpp
    debugwindow.cpp
    decoratedbasket.cpp
    diskerrordialog.cpp
    file_metadata.cpp
    filter.cpp
    focusedwidgets.cpp
    formatimporter.cpp
    global.cpp
    gitwrapper.cpp
    htmlexporter.cpp
    history.cpp
    kcolorcombo2.cpp
    kgpgme.cpp
    likeback.cpp
    linklabel.cpp
    newbasketdialog.cpp
    note.cpp
    notecontent.cpp
    notedrag.cpp
    noteedit.cpp
    notefactory.cpp
    noteselection.cpp
    password.cpp
    regiongrabber.cpp
    settings.cpp
    softwareimporters.cpp
    systemtray.cpp
    tag.cpp
    tagsedit.cpp
    transparentwidget.cpp
    tools.cpp
    variouswidgets.cpp
    xmlwork.cpp
)

set(PIMO_CPP "") # One of the generated files

IF(HAVE_NEPOMUK)
  include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src" ${NEPOMUK_INCLUDES} ${SOPRANO_INCLUDE_DIR})
  include(SopranoAddOntology)
  soprano_add_ontology(basketcommon_LIB_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nie.trig "NIE" "Soprano::Vocabulary" "trig")
  soprano_add_ontology(basketcommon_LIB_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/nie/nfo.trig "NFO" "Soprano::Vocabulary" "trig")
  soprano_add_ontology(basketcommon_LIB_SRCS ${SHAREDDESKTOPONTOLOGIES_ROOT_DIR}/pimo/pimo.trig "PIMO" "Soprano::Vocabulary" "trig")
  
  list(LENGTH basketcommon_LIB_SRCS SRCS_LENGTH)
  math(EXPR SRCS_LENGTH "${SRCS_LENGTH} - 1")
  list(GET basketcommon_LIB_SRCS ${SRCS_LENGTH} PIMO_CPP) # PIMO_CPP = last element
  
  list(INSERT basketcommon_LIB_SRCS 10 nepomukintegration.cpp)
ENDIF(HAVE_NEPOMUK)

qt5_wrap_ui(basketcommon_LIB_SRCS passwordlayout.ui basketproperties.ui)

QT5_ADD_DBUS_ADAPTOR(basketcommon_LIB_SRCS org.basket.BNPView.xml bnpview.h BNPView)

QT5_ADD_RESOURCES(basketcommon_LIB_SRCS ../basket.qrc)

add_library(basketcommon SHARED ${basketcommon_LIB_SRCS})

target_link_libraries(basketcommon
  ${PHONON_LIBRARY}
  ${GPGME_VANILLA_LIBRARIES}
  KF5::Archive
  KF5::ConfigWidgets
  KF5::CoreAddons
  KF5::Crash
  KF5::DBusAddons
  KF5::FileMetaData
  KF5::GlobalAccel
  KF5::GuiAddons
  KF5::I18n
  KF5::IconThemes
  KF5::KCMUtils
  KF5::KIOWidgets
  KF5::Notifications
  KF5::Parts
  KF5::Style
  KF5::TextWidgets
  KF5::WindowSystem
  KF5::XmlGui
)

if(Q_WS_X11)
    target_link_libraries(basketcommon ${X11_X11_LIB})
endif(Q_WS_X11)

IF(HAVE_NEPOMUK)
  target_link_libraries(basketcommon ${NEPOMUK_LIBRARIES} ${SOPRANO_LIBRARIES})
ENDIF(HAVE_NEPOMUK)

set_target_properties(basketcommon PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION})

install(TARGETS basketcommon DESTINATION ${LIB_INSTALL_DIR})

# Add unit tests after all variables have been set.
# If we save target_link_libraries to a variable, we can reuse it too

# Soprano files must exist, otherwise tests won't build
set(FILES_GENERATED (EXISTS ${PIMO_CPP}))
if(${FILES_GENERATED})
  add_subdirectory(tests)
endif(${FILES_GENERATED})

########### next target ###############

set(basket_SRCS main.cpp mainwindow.cpp application.cpp)

add_executable(basket ${basket_SRCS})
target_link_libraries(basket
  basketcommon)

IF(LIBGIT2_FOUND)
  target_link_libraries(basketcommon ${LIBGIT2_LIBRARIES})
  target_link_libraries(basket ${LIBGIT2_LIBRARIES})
ENDIF(LIBGIT2_FOUND)

install(TARGETS basket DESTINATION ${BIN_INSTALL_DIR})

########### next target ###############

set(kcm_basket_PART_SRCS kcm_basket.cpp)

add_library(kcm_basket MODULE ${kcm_basket_PART_SRCS})

target_link_libraries(kcm_basket basketcommon)

install(TARGETS kcm_basket DESTINATION ${PLUGIN_INSTALL_DIR})

if(BUILD_KPARTS)

########### next target ###############

set(basketpart_PART_SRCS basket_part.cpp)

set_target_properties(basketpart PROPERTIES PREFIX "${CMAKE_SHARED_LIBRARY_PREFIX}")
add_library(basketpart MODULE ${basketpart_PART_SRCS})

target_link_libraries(basketpart basketcommon kparts kdeui)

install(TARGETS basketpart DESTINATION ${PLUGIN_INSTALL_DIR})

set(kontact_basket_PLUGIN_SRCS basket_plugin.cpp)

add_library(kontact_basketplugin MODULE ${kontact_basket_PLUGIN_SRCS})

qt5_add_dbus_interfaces(kontact_basket_PLUGIN_SRCS org.kde.basket.basket.xml)

add_dependencies(kontact_basketplugin basket)

target_link_libraries(kontact_basketplugin basketcommon)

install(TARGETS kontact_basketplugin DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES basket_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kontact)

########### install files ###############

install(FILES  basket_part.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/basket)
install(FILES  basket_part.desktop DESTINATION ${SERVICES_INSTALL_DIR})

endif(BUILD_KPARTS)

install(FILES  basket.desktop DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES  basket_config_general.desktop  	basket_config_baskets.desktop  	basket_config_new_notes.desktop  	basket_config_notes_appearance.desktop  	basket_config_apps.desktop  	DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  basketui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/basket)

ecm_install_icons(${ICON_INSTALL_DIR})


