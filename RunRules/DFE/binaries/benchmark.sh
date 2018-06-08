#!/bin/bash

#configuring DFE (takes time)
./Vectors -in she_4k.ppm -out test.ppm 

./Vectors -in testImages/comic_hd.ppm -out outImages/OUTcomic_hd.ppm -benchmark -check -export
./Vectors -in testImages/god_hd.ppm -out outImages/OUTgod_hd.ppm -benchmark -check -export
./Vectors -in testImages/bat_fullhd.ppm -out outImages/OUTbat_fullhd.ppm -benchmark -check -export
./Vectors -in testImages/max_fullhd.ppm -out outImages/OUTmax_fullhd.ppm -benchmark -check -export
./Vectors -in testImages/zebra_qhd.ppm -out outImages/OUTzebra_qhd.ppm -benchmark -check -export
./Vectors -in testImages/1234_qhd.ppm -out outImages/OUT1234_qhd.ppm -benchmark -check -export
./Vectors -in testImages/she_4k.ppm -out outImages/OUTshe_4k.ppm -benchmark -check -export
./Vectors -in testImages/got.ppm -out outImages/OUTgot_4k.ppm -benchmark -check -export
