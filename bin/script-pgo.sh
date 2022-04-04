#!/usr/bin/env bash
for i in ../*/{,*/}*.inp; do ./epanetpgo $i $i.out; done
