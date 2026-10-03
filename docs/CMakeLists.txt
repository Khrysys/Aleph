add_custom_target(aleph_dirhtml_docs ALL
    COMMAND sphinx-build -M dirhtml ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS 
        "src/advanced/chess.md"
        "src/advanced/index.md"
        "src/appendix/acknowledgements.md"
        "src/basic/index.md"
        "src/basic/what-is-aleph.md"
        "src/developer/adr/0001-immutable-chess-objects.md"
        "src/developer/adr/0002-search-contempt-mcts.md"
        "src/developer/adr/0003-cross-attention-transformer.md"
        "src/developer/adr/0004-feed-forward-network-removal.md"
        "src/developer/adr/0005-smolgen.md"
        "src/developer/adr/0006-activation-function.md"
        "src/developer/adr/0007-sprt-network-promotion.md"
        "src/developer/adr/0008-supervised-pretraining.md"
        "src/developer/adr/index.md"
        "src/developer/building.md"
        "src/developer/style.md"
        "src/index.md"
)

if(Aleph_INSTALL)
    include(GNUInstallDirs)
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dirhtml DESTINATION "${CMAKE_INSTALL_DOCDIR}")
endif()