# -*- mode: sh -*- ###########################
# Qwt Widget Library
# Copyright (C) 1997   Josef Wilgen
# Copyright (C) 2002   Uwe Rathmann
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the Qwt License, Version 1.0
##############################################

# pro file for building the makefile for qwt
#
# Please read the instructions in INSTALL, if you don�t know
# how to use it.
#
# HEADERS/SOURCES are separated into plot
# relevant files and others. If you are 
# interested in QwtPlot only, you can remove
# the lists at the end of this file.

TARGET            = qwt

VERSION      = 5.0.0

# Only one of the following flags is allowed !
#CONFIG           += qtopia

CONFIG           += dll
include (../config.pri)

QWTPATH = $$TERRALIBPATH/src/qwt

CONFIG           += thread
CONFIG           += warn_on 

# Only one of the following flags is allowed !
CONFIG           += qt

INCLUDEPATH      += $$QWTPATH/include
DEPENDPATH       += $$QWTPATH/include $$QWTPATH

win32:dll:DEFINES    += QT_DLL QWT_DLL QWT_MAKEDLL
MOC_DIR = $$TERRALIBPATH/moc/qwt
HEADERS = \
    $$QWTPATH/include/qwt.h \
    $$QWTPATH/include/qwt_abstract_scale_draw.h \
    $$QWTPATH/include/qwt_array.h \
    $$QWTPATH/include/qwt_data.h \
    $$QWTPATH/include/qwt_double_interval.h \
    $$QWTPATH/include/qwt_double_rect.h \
    $$QWTPATH/include/qwt_dyngrid_layout.h \
    $$QWTPATH/include/qwt_event_pattern.h \
    $$QWTPATH/include/qwt_global.h \
    $$QWTPATH/include/qwt_interval_data.h \
    $$QWTPATH/include/qwt_layout_metrics.h \
    $$QWTPATH/include/qwt_legend.h \
    $$QWTPATH/include/qwt_legend_item.h \
    $$QWTPATH/include/qwt_math.h \
    $$QWTPATH/include/qwt_painter.h \
    $$QWTPATH/include/qwt_paint_buffer.h \
    $$QWTPATH/include/qwt_picker.h \
    $$QWTPATH/include/qwt_picker_machine.h \
    $$QWTPATH/include/qwt_plot.h \
    $$QWTPATH/include/qwt_plot_curve.h \
    $$QWTPATH/include/qwt_plot_dict.h \
    $$QWTPATH/include/qwt_plot_grid.h \
    $$QWTPATH/include/qwt_plot_layout.h \
    $$QWTPATH/include/qwt_plot_marker.h \
    $$QWTPATH/include/qwt_plot_printfilter.h \
    $$QWTPATH/include/qwt_plot_item.h \
    $$QWTPATH/include/qwt_plot_canvas.h \
    $$QWTPATH/include/qwt_plot_picker.h \
    $$QWTPATH/include/qwt_plot_zoomer.h \
    $$QWTPATH/include/qwt_rect.h \
    $$QWTPATH/include/qwt_round_scale_draw.h \
    $$QWTPATH/include/qwt_scale_widget.h \
    $$QWTPATH/include/qwt_scale_div.h \
    $$QWTPATH/include/qwt_scale_draw.h \
    $$QWTPATH/include/qwt_scale_engine.h \
    $$QWTPATH/include/qwt_scale_map.h \
    $$QWTPATH/include/qwt_spline.h \
    $$QWTPATH/include/qwt_symbol.h \
    $$QWTPATH/include/qwt_text_engine.h \
    $$QWTPATH/include/qwt_text_label.h \
    $$QWTPATH/include/qwt_text.h

SOURCES = \
    $$QWTPATH/qwt_abstract_scale_draw.cpp \
    $$QWTPATH/qwt_data.cpp \
    $$QWTPATH/qwt_double_interval.cpp \
    $$QWTPATH/qwt_double_rect.cpp \
    $$QWTPATH/qwt_dyngrid_layout.cpp \
    $$QWTPATH/qwt_event_pattern.cpp \
    $$QWTPATH/qwt_interval_data.cpp \
    $$QWTPATH/qwt_layout_metrics.cpp \
    $$QWTPATH/qwt_legend.cpp \
    $$QWTPATH/qwt_legend_item.cpp \
    $$QWTPATH/qwt_math.cpp \
    $$QWTPATH/qwt_painter.cpp \
    $$QWTPATH/qwt_paint_buffer.cpp \
    $$QWTPATH/qwt_picker.cpp \
    $$QWTPATH/qwt_picker_machine.cpp \
    $$QWTPATH/qwt_plot.cpp \
    $$QWTPATH/qwt_plot_axis.cpp \
    $$QWTPATH/qwt_plot_curve.cpp \
    $$QWTPATH/qwt_plot_dict.cpp \
    $$QWTPATH/qwt_plot_grid.cpp \
    $$QWTPATH/qwt_plot_item.cpp \
    $$QWTPATH/qwt_plot_print.cpp \
    $$QWTPATH/qwt_plot_marker.cpp \
    $$QWTPATH/qwt_plot_layout.cpp \
    $$QWTPATH/qwt_plot_printfilter.cpp \
    $$QWTPATH/qwt_plot_canvas.cpp \
    $$QWTPATH/qwt_plot_picker.cpp \
    $$QWTPATH/qwt_plot_zoomer.cpp \
    $$QWTPATH/qwt_rect.cpp \
    $$QWTPATH/qwt_round_scale_draw.cpp \
    $$QWTPATH/qwt_scale_widget.cpp \
    $$QWTPATH/qwt_scale_div.cpp \
    $$QWTPATH/qwt_scale_draw.cpp \
    $$QWTPATH/qwt_scale_engine.cpp \
    $$QWTPATH/qwt_scale_map.cpp \
    $$QWTPATH/qwt_spline.cpp \
    $$QWTPATH/qwt_symbol.cpp \
    $$QWTPATH/qwt_text_engine.cpp \
    $$QWTPATH/qwt_text_label.cpp \
    $$QWTPATH/qwt_text.cpp 
 
##############################################
# If you are interested in the plot widget
# only, you can remove the following 
# HEADERS/SOURCES lists.
##############################################

HEADERS += \
    $$QWTPATH/include/qwt_abstract_slider.h \
    $$QWTPATH/include/qwt_abstract_scale.h \
    $$QWTPATH/include/qwt_arrow_button.h \
    $$QWTPATH/include/qwt_analog_clock.h \
    $$QWTPATH/include/qwt_compass.h \
    $$QWTPATH/include/qwt_compass_rose.h \
    $$QWTPATH/include/qwt_counter.h \
    $$QWTPATH/include/qwt_dial.h \
    $$QWTPATH/include/qwt_dial_needle.h \
    $$QWTPATH/include/qwt_double_range.h \
    $$QWTPATH/include/qwt_knob.h \
    $$QWTPATH/include/qwt_slider.h \
    $$QWTPATH/include/qwt_thermo.h \
    $$QWTPATH/include/qwt_wheel.h

SOURCES += \
    $$QWTPATH/qwt_abstract_slider.cpp \
    $$QWTPATH/qwt_abstract_scale.cpp \
    $$QWTPATH/qwt_arrow_button.cpp \
    $$QWTPATH/qwt_analog_clock.cpp \
    $$QWTPATH/qwt_compass.cpp \
    $$QWTPATH/qwt_compass_rose.cpp \
    $$QWTPATH/qwt_counter.cpp \
    $$QWTPATH/qwt_dial.cpp \
    $$QWTPATH/qwt_dial_needle.cpp \
    $$QWTPATH/qwt_double_range.cpp \
    $$QWTPATH/qwt_knob.cpp \
    $$QWTPATH/qwt_slider.cpp \
    $$QWTPATH/qwt_thermo.cpp \
    $$QWTPATH/qwt_wheel.cpp
