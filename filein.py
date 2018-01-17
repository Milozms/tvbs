#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Top Block
# Generated: Mon Jan 15 20:32:11 2018
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from PyQt4 import Qt
from gnuradio import blocks
from gnuradio import eng_notation
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from optparse import OptionParser
import sys
from gnuradio import qtgui
import os
from multiprocessing import Process
import ctypes
import threading

class top_block(gr.top_block, Qt.QWidget):

    def __init__(self, filename, fd):
        gr.top_block.__init__(self, "Top Block")
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Top Block")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except:
            pass
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("GNU Radio", "top_block")
        self.restoreGeometry(self.settings.value("geometry").toByteArray())

        ##################################################
        # Variables
        ##################################################
        self.sample_rate = sample_rate = (6000000.0 * 8) / 7
        self.channel_hz = channel_hz = 6e6

        ##################################################
        # Blocks
        ##################################################
        self.blocks_file_source_0 = blocks.file_source(gr.sizeof_char*1512, filename, False)
        self.blocks_file_sink_0_0_0 = blocks.file_descriptor_sink(gr.sizeof_char*1512, fd)
        # self.blocks_file_sink_0_0_0.set_unbuffered(False)

        ##################################################
        # Connections
        ##################################################
        self.connect((self.blocks_file_source_0, 0), (self.blocks_file_sink_0_0_0, 0))

    def closeEvent(self, event):
        self.settings = Qt.QSettings("GNU Radio", "top_block")
        self.settings.setValue("geometry", self.saveGeometry())
        event.accept()

    def get_sample_rate(self):
        return self.sample_rate

    def set_sample_rate(self, sample_rate):
        self.sample_rate = sample_rate

    def get_channel_hz(self):
        return self.channel_hz

    def set_channel_hz(self, channel_hz):
        self.channel_hz = channel_hz


def decode_proc(fd1, fd2):
    libtvbs = ctypes.cdll.LoadLibrary('/home/zms/CLionProjects/tvbs/libtvbs.so')
    libtvbs.decode_main(fd1, fd2)

def main(top_block_cls=top_block, options=None):

    from distutils.version import StrictVersion
    if StrictVersion(Qt.qVersion()) >= StrictVersion("4.5.0"):
        style = gr.prefs().get_string('qtgui', 'style', 'raster')
        Qt.QApplication.setGraphicsSystem(style)
    qapp = Qt.QApplication(sys.argv)

    r1, w1 = os.pipe()
    r2, w2 = os.pipe()
    p = Process(target=decode_proc, args=(r1, r2,))
    
    tb1 = top_block_cls('/home/zms/tvbsdata/1231_ori_after_map_03.ts', w1)
    tb2 = top_block_cls('/home/zms/tvbsdata/1231_bs_after_map_03.ts', w2)
    p.start()
    tb1.start()
    tb2.start()

    def quitting():
        tb1.stop()
        tb1.wait()
        tb2.stop()
        tb2.wait()
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)
    qapp.exec_()


if __name__ == '__main__':
    main()
