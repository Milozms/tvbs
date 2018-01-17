import ctypes
libtvbs = ctypes.cdll.LoadLibrary('/Users/zms/CLionProjects/tvbs/libtvbs.so')
f1 = open('/Users/zms/CLionProjects/tvbs/cmake-build-debug/1231_ori_after_map_04.ts', 'rb')
f2 = open('/Users/zms/CLionProjects/tvbs/cmake-build-debug/1231_bs_after_map_04.ts', 'rb')
f1s = f1.read()
f2s = f2.read()
libtvbs.decode_xor(f1s, len(f1s), f2s, len(f2s), ctypes.c_double(0.02))
# g++ -o libtvbs.so -shared -fPIC tvbs_decode.cpp