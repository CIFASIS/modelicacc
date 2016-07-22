model trline  
  Modelica.Electrical.Analog.Sources.PulseVoltage pulsevoltage1;
  Modelica.Electrical.Analog.Basic.Resistor resistor1;
  Modelica.Electrical.Analog.Basic.Ground ground1;
  constant Integer N = 10;
  line_segment[N] line_segment1;
equation
  connect(pulsevoltage1.n, resistor1.n);
  for i in 1:N - 1 loop
    connect(line_segment1[i].pin1, line_segment1[i + 1].pin0);
  end for;
  connect(line_segment1[1].pin0, pulsevoltage1.p);
  connect(line_segment1[N].pin1, resistor1.p);
end trline;


model line_segment  
  Modelica.Electrical.Analog.Basic.Ground ground1;
  Modelica.Electrical.Analog.Basic.Resistor resistor1;
  Modelica.Electrical.Analog.Basic.Inductor inductor1;
  Modelica.Electrical.Analog.Basic.Capacitor capacitor1;
  Modelica.Electrical.Analog.Interfaces.Pin pin1;
  Modelica.Electrical.Analog.Interfaces.Pin pin0;
equation
  connect(pin0, resistor1.p);
  connect(pin1, capacitor1.p);
  connect(capacitor1.n, ground1.p);
  connect(inductor1.n, capacitor1.p);
  connect(resistor1.n, inductor1.p);
end line_segment;






package ModelicaServices  
  //extends Modelica.Icons.Package;

  package Machine  
   // extends Modelica.Icons.Package;
    final constant Real eps = 1.e-15;
    final constant Real small = 1.e-60;
    final constant Real inf = 1.e+60;
    final constant Integer Integer_inf = OpenModelica.Internal.Architecture.integerMax();
  end Machine;
end ModelicaServices;

package Modelica  
  //extends Modelica.Icons.Package;

  package Blocks  
   // extends Modelica.Icons.Package;

    package Interfaces  
    //  extends Modelica.Icons.InterfacesPackage;
      connector RealOutput = output Real;

      partial block SO  
       // extends Modelica.Blocks.Icons.Block;
        RealOutput y;
      end SO;

      partial block SignalSource  
        extends SO;
        parameter Real offset = 0;
        parameter .Modelica.SIunits.Time startTime = 0;
      end SignalSource;
    end Interfaces;

    package Sources  
      //extends Modelica.Icons.SourcesPackage;

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

    /*package Icons  
      extends Modelica.Icons.IconsPackage;

      partial block Block  end Block;
    end Icons;*/
  end Blocks;

  package Electrical  
    //extends Modelica.Icons.Package;

    package Analog  
     // extends Modelica.Icons.Package;

      package Basic  
       // extends Modelica.Icons.Package;

        model Ground  
          Interfaces.Pin p;
        equation
          p.v = 0;
        end Ground;

        model Resistor  
          parameter Modelica.SIunits.Resistance R(start = 1);
          parameter Modelica.SIunits.Temperature T_ref = 300.15;
          parameter Modelica.SIunits.LinearTemperatureCoefficient alpha = 0;
          extends Modelica.Electrical.Analog.Interfaces.OnePort;
          extends Modelica.Electrical.Analog.Interfaces.ConditionalHeatPort(T = T_ref);
          Modelica.SIunits.Resistance R_actual;
        equation
          assert(1 + alpha * (T_heatPort - T_ref) >= Modelica.Constants.eps, "Temperature outside scope of model!");
          R_actual = R * (1 + alpha * (T_heatPort - T_ref));
          v = R_actual * i;
          LossPower = v * i;
        end Resistor;

        model Capacitor  
          extends Interfaces.OnePort(v(start = 0));
          parameter .Modelica.SIunits.Capacitance C(start = 1);
        equation
          i = C * der(v);
        end Capacitor;

        model Inductor  
          extends Interfaces.OnePort(i(start = 0));
          parameter .Modelica.SIunits.Inductance L(start = 1);
        equation
          L * der(i) = v;
        end Inductor;
      end Basic;

      package Interfaces  
        //extends Modelica.Icons.InterfacesPackage;

        connector Pin  
          Modelica.SIunits.Voltage v annotation(unassignedMessage = "An electrical potential cannot be uniquely calculated.
            The reason could be that
            - a ground object is missing (Modelica.Electrical.Analog.Basic.Ground)
              to define the zero potential of the electrical circuit, or
            - a connector of an electrical component is not connected.");
          flow Modelica.SIunits.Current i annotation(unassignedMessage = "An electrical current cannot be uniquely calculated.
            The reason could be that
            - a ground object is missing (Modelica.Electrical.Analog.Basic.Ground)
              to define the zero potential of the electrical circuit, or
            - a connector of an electrical component is not connected.");
        end Pin;

        connector PositivePin  
          Modelica.SIunits.Voltage v annotation(unassignedMessage = "An electrical potential cannot be uniquely calculated.
            The reason could be that
            - a ground object is missing (Modelica.Electrical.Analog.Basic.Ground)
              to define the zero potential of the electrical circuit, or
            - a connector of an electrical component is not connected.");
          flow Modelica.SIunits.Current i annotation(unassignedMessage = "An electrical current cannot be uniquely calculated.
            The reason could be that
            - a ground object is missing (Modelica.Electrical.Analog.Basic.Ground)
              to define the zero potential of the electrical circuit, or
            - a connector of an electrical component is not connected.");
        end PositivePin;

        connector NegativePin  
          Modelica.SIunits.Voltage v annotation(unassignedMessage = "An electrical potential cannot be uniquely calculated.
            The reason could be that
            - a ground object is missing (Modelica.Electrical.Analog.Basic.Ground)
              to define the zero potential of the electrical circuit, or
            - a connector of an electrical component is not connected.");
          flow Modelica.SIunits.Current i annotation(unassignedMessage = "An electrical current cannot be uniquely calculated.
            The reason could be that
            - a ground object is missing (Modelica.Electrical.Analog.Basic.Ground)
              to define the zero potential of the electrical circuit, or
            - a connector of an electrical component is not connected.");
        end NegativePin;

        partial model OnePort  
          .Modelica.SIunits.Voltage v;
          .Modelica.SIunits.Current i;
          PositivePin p;
          NegativePin n;
        equation
          v = p.v - n.v;
          0 = p.i + n.i;
          i = p.i;
        end OnePort;

        partial model ConditionalHeatPort  
          parameter Boolean useHeatPort = false annotation(Evaluate = true, HideResult = true);
          parameter Modelica.SIunits.Temperature T = 293.15;
          Modelica.Thermal.HeatTransfer.Interfaces.HeatPort_a heatPort(T(start = T) = T_heatPort, Q_flow = -LossPower) if useHeatPort;
          Modelica.SIunits.Power LossPower;
          Modelica.SIunits.Temperature T_heatPort;
        equation
          if not useHeatPort then
            T_heatPort = T;
          end if;
        end ConditionalHeatPort;

        partial model VoltageSource  
          extends OnePort;
          parameter .Modelica.SIunits.Voltage offset = 0;
          parameter .Modelica.SIunits.Time startTime = 0;
          replaceable Modelica.Blocks.Interfaces.SignalSource signalSource(final offset = offset, final startTime = startTime);
        equation
          v = signalSource.y;
        end VoltageSource;
      end Interfaces;

      package Sources  
        //extends Modelica.Icons.SourcesPackage;

        model PulseVoltage  
          parameter .Modelica.SIunits.Voltage V(start = 1);
          parameter Real width(final min = Modelica.Constants.small, final max = 100, start = 50);
          parameter .Modelica.SIunits.Time period(final min = Modelica.Constants.small, start = 1);
          extends Interfaces.VoltageSource(redeclare Modelica.Blocks.Sources.Pulse signalSource(amplitude = V, width = width, period = period));
        end PulseVoltage;
      end Sources;
    end Analog;
  end Electrical;

  package Thermal  
    //extends Modelica.Icons.Package;

    package HeatTransfer  
      //extends Modelica.Icons.Package;

      package Interfaces  
      //  extends Modelica.Icons.InterfacesPackage;

        partial connector HeatPort  
          Modelica.SIunits.Temperature T;
          flow Modelica.SIunits.HeatFlowRate Q_flow;
        end HeatPort;

        connector HeatPort_a  
          extends HeatPort;
        end HeatPort_a;
      end Interfaces;
    end HeatTransfer;
  end Thermal;

  package Math  
  //  extends Modelica.Icons.Package;

    //package Icons  
   //   extends Modelica.Icons.IconsPackage;

   //   partial function AxisCenter  end AxisCenter;
//end Icons;

    function asin  
  //    extends Modelica.Math.Icons.AxisCenter;
      input Real u;
      output .Modelica.SIunits.Angle y;
      external "builtin" y = asin(u);
    end asin;

    function exp  
   //   extends Modelica.Math.Icons.AxisCenter;
      input Real u;
      output Real y;
      external "builtin" y = exp(u);
    end exp;
  end Math;

  package Constants  
  //  extends Modelica.Icons.Package;
    final constant Real pi = 2 * Math.asin(1.0);
    final constant Real eps = ModelicaServices.Machine.eps;
    final constant Real small = ModelicaServices.Machine.small;
    final constant .Modelica.SIunits.Velocity c = 299792458;
    final constant Real mue_0(final unit = "N/A2") = 4 * pi * 1.e-7;
  end Constants;


  package SIunits  
   //extends Modelica.Icons.Package;

    package Conversions  
      //extends Modelica.Icons.Package;

      package NonSIunits  
       // extends Modelica.Icons.Package;
        type Temperature_degC = Real(final quantity = "ThermodynamicTemperature", final unit = "degC");
      end NonSIunits;
    end Conversions;

    type Angle = Real(final quantity = "Angle", final unit = "rad", displayUnit = "deg");
    type Time = Real(final quantity = "Time", final unit = "s");
    type Velocity = Real(final quantity = "Velocity", final unit = "m/s");
    type Acceleration = Real(final quantity = "Acceleration", final unit = "m/s2");
    type Power = Real(final quantity = "Power", final unit = "W");
    type ThermodynamicTemperature = Real(final quantity = "ThermodynamicTemperature", final unit = "K", min = 0.0, start = 288.15, nominal = 300, displayUnit = "degC");
    type Temperature = ThermodynamicTemperature;
    type LinearTemperatureCoefficient = Real(final quantity = "LinearTemperatureCoefficient", final unit = "1/K");
    type HeatFlowRate = Real(final quantity = "Power", final unit = "W");
    type ElectricCurrent = Real(final quantity = "ElectricCurrent", final unit = "A");
    type Current = ElectricCurrent;
    type ElectricPotential = Real(final quantity = "ElectricPotential", final unit = "V");
    type Voltage = ElectricPotential;
    type Capacitance = Real(final quantity = "Capacitance", final unit = "F", min = 0);
    type Inductance = Real(final quantity = "Inductance", final unit = "H");
    type Resistance = Real(final quantity = "Resistance", final unit = "Ohm");
    type FaradayConstant = Real(final quantity = "FaradayConstant", final unit = "C/mol");
  end SIunits;
end Modelica;


