#!/usr/bin/env python
#
# A simple constraint-based resistance calculator

import pylab
import unittest
import scipy

class Circuit(object):
    
    def __init__(self, power, leds, transistor):
        self.power = power
        self.leds = leds
        self.transistor = transistor

    def calculate_resistors(self): 
        """ 
        maximize current to LED subjec to constraints of leds.max_current and
        transistor.max_current
        """
        ledresistance = {}
        for myled in self.leds:
            resistance = (self.power.volts -
                          myled.fwd_voltage)/(myled.max_current)
            ledresistance[myled.color] = resistance 

        return ledresistance


class Transistor(object):

    def __init__(self, func_gain, max_current):
        self.func_gain = func_gain
        self.max_current = max_current

    def fitpoly(self):
        """ 
        Modify this to produce a polynomial function which fits the gain data
        points provided by the transistor datasheet
        """
        xdata = scipy.linspace(0, 9, num=10)
        ydata = 0.5+xdata*2.0
        ydata += scipy.rand(10)
        polycoeffs = scipy.polyfit(xdata, ydata, 1)
        yfit = scipy.polyval(polycoeffs, xdata)
        pylab.plot(xdata, ydata, 'k.')
        pylab.plot(xdata, yfit, 'r-')

        return polycoeffs

class LED(object):

    def __init__(self, color, fwd_voltage, max_current):
        self.color = color
        self.fwd_voltage = fwd_voltage
        self.max_current = max_current

    def __str__(self):
        return "%s, %s, %s" % (self.color, self.fwd_voltage, self.max_current)

class Power(object):

    def __init__(self, volts, amps):
        self.volts = volts
        self.amps = amps


class CircuitTest(unittest.TestCase):

    def test_circuit(self):
        mypower = Power(5.0, 1.2)
        myleds = [ LED(*i) for i in (('red', 2.2, 0.150),
                                    ('green', 3.5, 0.150),
                                    ('blue', 3.5, 0.150)) ]
        mytrans = Transistor(None, 0.100)
        mycircuit = Circuit(mypower, myleds, mytrans)
        print map(str, myleds)
        print mycircuit.calculate_resistors()

    def test_highpower_circuit(self):
        mypower = Power(5.0, 1.2)
        myleds = [ LED(*i) for i in (('red', 2.5, 0.400),
                                    ('green', 3.4, 0.350),
                                    ('blue', 3.4, 0.350)) ]
        mytrans = Transistor(None, 0.100)
        mycircuit = Circuit(mypower, myleds, mytrans)
        print map(str, myleds)
        print mycircuit.calculate_resistors()


if __name__ == '__main__':
    unittest.main()
