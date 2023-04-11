#!/bin/bash 

pipedgen=pipedgen
fastcap=fastcap

$pipedgen -nact1 -c1 0.5 0 0 -c2 0 10 0 -c3 0 0 0.5 >ct1.geo
$pipedgen -nact2 -c1 0.5 0 0 -c2 0 10 0 -c3 0 0 0.5 >ct2.geo
$pipedgen -nacb -nt20 -c1 10 0 0 -c2 0 10 0 -c3 0 0 0.5 >cb.geo

rm all.lst
echo "* Interdigit mesh" >>all.lst

echo "C ct1.geo 1.0 1.25 0.0 2.5 +" >>all.lst
echo "C ct1.geo 1.0 3.25 0.0 2.5 +" >>all.lst
echo "C ct1.geo 1.0 5.25 0.0 2.5 +" >>all.lst
echo "C ct1.geo 1.0 7.25 0.0 2.5" >>all.lst

echo "C ct2.geo 1.0 2.25 0.0 2.5 +" >>all.lst
echo "C ct2.geo 1.0 4.25 0.0 2.5 +" >>all.lst
echo "C ct2.geo 1.0 6.25 0.0 2.5 +" >>all.lst
echo "C ct2.geo 1.0 8.25 0.0 2.5" >>all.lst

echo "C cb.geo 1.0 0.0 0.0 0" >>all.lst

$fastcap -m -lall.lst
$fastcap -lall.lst

