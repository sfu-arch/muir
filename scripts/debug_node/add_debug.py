#!/usr/bin/env python3
import argparse
import json
import sys
import re
import os.path
from os import path


def main(argv):
    parser = argparse.ArgumentParser(description='Short sample app')

    parser.add_argument('-s', '--scala-file', action='store', dest='input', required=True, help = 'input scala file')
    parser.add_argument('-c', '--config-file', action='store', dest='config', required=True, help = 'input config file')

    args = parser.parse_args(argv)

    if path.isfile(args.input) and args.input.lower().endswith('scala'):
        print(args.input)
    else:
        print("Input scala file is not valid!")
        return
    
    if path.isfile(args.config) and args.config.lower().endswith('json'):
        print(args.config)

    else:
        print("Input config file is not valid!")
        return

    with open(args.config) as json_file:
        config = json.load(json_file)


    new_file = args.input.replace('.scala', '-debug.scala')
    debug_file = open(new_file,"w+")

    with open(args.input) as input_scala:
        for line in input_scala:
            if line.lstrip().startswith('val'):
                reg = re.findall(r'val (.+?) = .* ID = (.+?), .*', line.lstrip())

                if reg:
                    name = reg[0][0]
                    id = reg[0][1]

                    for node in config['module']['node']:
                        if node['name'] == name:
                            if node['debug'].lower() == 'true':
                                debug_file.write(line.replace('Debug = false', 'Debug = true'))
                            else:
                                debug_file.write(line)

                
            else:
                debug_file.write(line)
    
    debug_file.close()





if __name__ == "__main__":
   main(sys.argv[1:])