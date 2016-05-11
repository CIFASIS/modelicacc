
model HeatingSystem
      "Declarative model of a heating system - continuous-time dynamics"
      import SI = Modelica.SIunits;
      constant Real pi = 3.14;
      parameter Integer N = 3 "Number of heated units";
      parameter SI.HeatCapacity Cu[N] = (ones(N)+ linspace(0,1.348,N))*1e7
        "Heat capacity of heated units";
      parameter SI.HeatCapacity Cd = 2e6*N
        "Heat capacity of distribution circuit";
      parameter SI.ThermalConductance Gh = 200
        "Thermal conductance of heating elements";
      parameter SI.ThermalConductance Gu = 150
        "Thermal conductance of heated units to the atmosphere";
      parameter SI.Power Qmax = N*3000
        "Maximum power output of heat generation unit";
      parameter SI.TemperatureDifference Teps = 0.5
        "Threshold of heated unit temperature controllers";
      parameter SI.Temperature Td0 = 343.15
        "Set point of distribution circuit temperature";
      parameter SI.Temperature Tu0 = 293.15 "Heated unit temperature set point";
      parameter Real Kp = Qmax/4
        "Proportional gain of heat generation unit temperature controller";
      parameter Real a = 50 "Gain of the histeresis function";
      parameter Real b = 15 "Slope of the saturation function at the origin";

      // State variables
      Real Td(start = Td0, fixed = true)
        "Temperature of the fluid in the distribution system";
      Real Tu[N](each start = Tu0, each fixed = true)
        "Temperature of individual heated units";
      Real x[N](each start = -0.5, each fixed = true)
        "States of heated units temperature controllers";

      // Time-varying prescribed signals
      Real Text "External temperature";

      // Other intermediate algebraic variables
      Real Que[N] "Heat flows from heated units to the outside";
      Real Qh[N] "Heat flows to each heated unit";
      Real Qd "Heat flow to the distribution system";
      Real u[N] "Control signals for heaters";
    equation
      Text = 278.15 + 8*sin(2*pi*time/86400);
      Cd*der(Td) =Qd - sum(Qh);
      Qd = sat(Kp*(Td0-Td),0, Qmax);
      for i in 1:N loop
        Qh[i] = Gh*(Td - Tu[i])*u[i];
        Que[i] =  Gu*(Tu[i] - Text);
        Cu[i]*der(Tu[i]) = Qh[i] - Que[i];
        der(x[i]) = a*hist(x[i], Tu0 - Tu[i], Teps);
        u[i] = sat(b*x[i], -0.5, 0.5)+0.5;
      end for;
      annotation (Documentation(info="<html>
    <p>This model represents a district heating system with N heated units, supplied by a heat distribution system. Each heated unit is described as a lumped-parameter energy balance with one temperature Tu, losing heat to the ambient, whose temperature Ta varies sinusoidally with a period of one day. The heat distribution system is also described by a large lumped thermal capacitance with a single temperature Td.</p>
    <p>The temperature of each unit is controlled by an on-off controller with hysteresis, which determines the conductance between the heating medium and the unit temperature, mimicking the behaviour of a fan-coil heater. The controller dynamics is nonlinear and subject to bifurcations. If the temperature lies outside of the hysteresis interval, the state x of the controller has one stable state, which is then converted to a 0-1 output by a nonlinear output function sat(). Inside the hysteresis interval, there are two stable states with an unstable state in-between. When the temperature gets higher or smaller than the hysteresis threshold, the current stable state collides with the unstable state and disappears, so that the state x undergoes a fast transition to the other equilibrium.</p>
    <p>The unit temperature controller dynamics is extremely stiff, due to the strong nonlinearity of the sat() function and to the wide changes of eigenvalues (including positive values!) during the transition to the new stable state.</p>
    <p>The distribution system temperature is instead controlled by a simple proportional controller with a relatively low bandwidth, so that the system can be used as a buffer between the heat supply system and the consumers on the network. Smooth saturations on the control variable are present, to avoid negative heat flows or too high heat flows that would be unrealistic.</p>
    <p>The heat capacitances of the units are slightly different from each other, so that the local on-off controller transitions take place asynchronously. Since the large thermal capacitance of the storage system decouples the behaviour of each unit over the time intervals relevant for the simulation of the on-off controllers, this system can be integrated much more efficiently if adaptive multi-rate algorithms are employed, as explained in <a href=\"http://dx.doi.org/10.1145/2666202.2666214\">this paper</a>.</p>
    <p>The order of the system is 2*N+1.</p>
    </html>"));
end HeatingSystem;