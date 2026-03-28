idf := require('idf.py')

build:
    @{{idf}} build

flash port="/dev/ttyUSB0": build
    @{{idf}} flash -p {{port}}

monitor port="/dev/ttyUSB0":
    @{{idf}} monitor -p {{port}}

config:
    @{{idf}} menuconfig

clean:
    @{{idf}} clean

fullclean:
    @{{idf}} fullclean
