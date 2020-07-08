package Modelica  

  package Blocks  

    package Interfaces  
      connector RealInput = input Real;
      connector RealOutput = output Real;

      partial block SO  
        RealOutput y;
      end SO;

      partial block SI2SO  
        RealInput u1;
        RealInput u2;
        RealOutput y;
      end SI2SO;
    end Interfaces;

    package Math  
		
	  model Example  
		Modelica.Blocks.Math.Add add1;
		Modelica.Blocks.Math.Product product1;
		Modelica.Blocks.Sources.Sine sine1;
		  Modelica.Blocks.Sources.Pulse pulse1;
		  Modelica.Blocks.Sources.Cosine cosine1;
		equation
		  connect(cosine1.y, product1.u1);
		  connect(pulse1.y, product1.u2);
		  connect(product1.y, add1.u2);
		  connect(sine1.y, add1.u1);
		  connect(product1.y, add1.u2);
		end Example;		
		
      block Add  
        extends Modelica.Blocks.Interfaces.SI2SO;
        parameter Real k1 = +1;
        parameter Real k2 = +1;
      equation
        y = k1 * u1 + k2 * u2;
      end Add;

      block Product  
        extends Modelica.Blocks.Interfaces.SI2SO;
      equation
        y = u1 * u2;
      end Product;
    end Math;

    package Sources  

      block Sine  
        parameter Real amplitude = 1;
        parameter Modelica.SIunits.Frequency freqHz(start = 1);
        parameter Modelica.SIunits.Angle phase = 0;
        parameter Real offset = 0;
        parameter Modelica.SIunits.Time startTime = 0;
        extends Modelica.Blocks.Interfaces.SO;
      protected
        constant Real pi = Modelica.Constants.pi;
      equation
        y = offset + (if time < startTime then 0 else amplitude * Modelica.Math.sin(2 * pi * freqHz * (time - startTime) + phase));
      end Sine;

      block Cosine  
        parameter Real amplitude = 1;
        parameter Modelica.SIunits.Frequency freqHz(start = 1);
        parameter Modelica.SIunits.Angle phase = 0;
        parameter Real offset = 0;
        parameter Modelica.SIunits.Time startTime = 0;
        extends Modelica.Blocks.Interfaces.SO;
      protected
        constant Real pi = Modelica.Constants.pi;
      equation
        y = offset + (if time < startTime then 0 else amplitude * ModelicaMath.cos(2 * pi * freqHz * (time - startTime) + phase));
      end Cosine;

      block Pulse  
        parameter Real amplitude = 1;
        parameter Real width(final min = Modelica.Constants.small, final max = 100) = 50;
        parameter Modelica.SIunits.Time period(final min = Modelica.Constants.small, start = 1);
        parameter Integer nperiod = -1;
        parameter Real offset = 0;
        parameter Modelica.SIunits.Time startTime = 0;
        extends Modelica.Blocks.Interfaces.SO;
      protected
        Modelica.SIunits.Time T_width = period * width / 100;
        Modelica.SIunits.Time T_start;
        Integer count;
      initial algorithm
        count := integer((time - startTime) / period);
        T_start := startTime + count * period;
      equation
        when integer((time - startTime) / period) > pre(count) then
          count = pre(count) + 1;
          T_start = time;
        end when;
        y = offset + (if time < startTime or nperiod == 0 or nperiod > 0 and count >= nperiod then 0 else if time < T_start + T_width then amplitude else 0);
      end Pulse;
    end Sources;

    
  end Blocks;

  package Math  

    function sin  
      input Modelica.SIunits.Angle u;
      output Real y;
      external "builtin" y = sin(u);
    end sin;

    function cos  
      input Modelica.SIunits.Angle u;
      output Real y;
      external "builtin" y = cos(u);
    end cos;

    function asin  
      input Real u;
      output Modelica.SIunits.Angle y;
      external "builtin" y = asin(u);
    end asin;

    function exp  
      input Real u;
      output Real y;
      external "builtin" y = exp(u);
    end exp;
  end Math;

  package Constants  
    final constant Real pi = 2 * Math.asin(1.0);
    final constant Real small = ModelicaServices.Machine.small;
    final constant Modelica.SIunits.Velocity c = 299792458;
    final constant Real mue_0(final unit = "N/A2") = 4 * pi * 1e-07;
  end Constants;

  package SIunits  

    package Conversions  

      package NonSIunits  
        type Temperature_degC = Real(final quantity = "ThermodynamicTemperature", final unit = "degC");
      end NonSIunits;
    end Conversions;

    type Angle = Real(final quantity = "Angle", final unit = "rad", displayUnit = "deg");
    type Time = Real(final quantity = "Time", final unit = "s");
    type Velocity = Real(final quantity = "Velocity", final unit = "m/s");
    type Acceleration = Real(final quantity = "Acceleration", final unit = "m/s2");
    type Frequency = Real(final quantity = "Frequency", final unit = "Hz");
    type FaradayConstant = Real(final quantity = "FaradayConstant", final unit = "C/mol");
  end SIunits;
end Modelica;
