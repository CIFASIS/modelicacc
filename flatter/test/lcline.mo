
package ModelicaServices  

  package Machine  
    final constant Real eps = 1.e-15;
    final constant Real small = 1.e-60;
    final constant Real inf = 1.e+60;
    final constant Integer Integer_inf = OpenModelica.Internal.Architecture.integerMax();
  end Machine;
end ModelicaServices;

package Modelica  

  package Electrical  

    package Analog  

      package Basic  

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
          //assert(1 + alpha * (T_heatPort - T_ref) >= Modelica.Constants.eps, "Temperature outside scope of model!");
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
      end Interfaces;

      package Sources  

        model ConstantVoltage  
          parameter .Modelica.SIunits.Voltage V(start = 1);
          extends Interfaces.OnePort;
        equation
          v = V;
        end ConstantVoltage;
      end Sources;
    end Analog;
  end Electrical;

  package Thermal  

    package HeatTransfer  

      package Interfaces  

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


    function asin  
      input Real u;
      output .Modelica.SIunits.Angle y;
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
    final constant Real eps = ModelicaServices.Machine.eps;
    final constant .Modelica.SIunits.Velocity c = 299792458;
    final constant Real mue_0(final unit = "N/A2") = 4 * pi * 1.e-7;
  end Constants;

  package SIunits  

    package Conversions  

      package NonSIunits  
        type Temperature_degC = Real(final quantity = "ThermodynamicTemperature", final unit = "degC");
      end NonSIunits;
    end Conversions;

    type Angle = Real(final quantity = "Angle", final unit = "rad", displayUnit = "deg");
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
model lcline  
  import Analog = Modelica.Electrical.Analog;
  model lcsection  
    Modelica.Electrical.Analog.Interfaces.Pin pin2;
    Modelica.Electrical.Analog.Basic.Inductor i;
    Modelica.Electrical.Analog.Interfaces.Pin pin;
    Modelica.Electrical.Analog.Interfaces.Pin pin1;
    Modelica.Electrical.Analog.Basic.Capacitor c;
  equation
    connect(c.p, pin1);
    connect(pin, i.p);
    connect(i.n, c.p);
    connect(c.n, pin2);
  end lcsection;

  Modelica.Electrical.Analog.Basic.Resistor r;
  Modelica.Electrical.Analog.Basic.Ground g;
  constant Integer N = 10;
  lcsection[N] lc;
  Modelica.Electrical.Analog.Sources.ConstantVoltage s;
equation
  connect(r.n, g.p);
  connect(s.n, g.p);
  connect(s.p, lc[1].pin);
  connect(r.p, lc[N].pin1);
  connect(g.p, lc[N].pin2);
  for i in 1:N - 1 loop
    connect(g.p, lc[i].pin2);
    connect(lc[i].pin1, lc[i + 1].pin);
  end for;
end lcline;

