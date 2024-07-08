# -*- coding: utf-8 -*-
# @Author: Jelly
# @Email: jellyHello@163.com
# @Time: 3/31/23 5:53 PM

import os
import configparser


def param_parse():
    # read ./conf/parameter.ini
    base_dir = os.path.dirname(os.path.abspath('__file__'))
    configPath = os.path.join(base_dir, 'conf/system/parameter.ini')
    config = configparser.ConfigParser()
    config.read(configPath)
    return config


if __name__ == '__main__':
    config = param_parse()
    sampleInterval = config.get("base", "sample_interval")

