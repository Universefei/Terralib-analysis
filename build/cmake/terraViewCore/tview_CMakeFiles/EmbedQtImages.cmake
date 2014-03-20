# This script adds images as source
# (c) Frederico A. Bedê, june 2011
#
# QT3 Macro for embedding image files into source
#
# EMBED_TARGET  = binary to link to, i.e. sbvd_manager
# EMBED_SOURCE_OUTPUT = outputted .cpp file
# SOURCE_IMAGES = list of images to embed
#

Macro(QT_EMBED_IMAGES EMBED_TARGET EMBED_SOURCE_OUTPUT SOURCE_IMAGES)
    add_Custom_Command( OUTPUT ${EMBED_SOURCE_OUTPUT}
                        COMMAND ${QT_UIC_EXECUTABLE}
                                -embed ${EMBED_TARGET}
                                ${${SOURCE_IMAGES}}
                                -o ${EMBED_SOURCE_OUTPUT} )
EndMacro(QT_EMBED_IMAGES EMBED_TARGET EMBED_SOURCE_OUTPUT SOURCE_IMAGES)
