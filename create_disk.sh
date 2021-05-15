#!/bin/bash
bximage -mode=create -hd=60M -sectsize=512 -q hd60M.img
mv hd60M.img ./disk/hd60M.img
