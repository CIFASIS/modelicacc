model CocurrentHeatExchangerEquations
  "cocurrent heat exchanger implemented by equations"
  import Modelica.SIunits;
  parameter Real L=10 "length of the channels";
  constant Integer N=20 "number of nodes";
  parameter Real wB=1 "mass flow rate of fluid B";
  parameter Real areaA=5e-5 "cross sectional area of channel A";
  parameter Real areaB=5e-5 "cross sectional area of channel B";
  parameter Real rhoA=1000 "density of fluid A";
  parameter Real rhoB=1000 "density of fluid B";
  parameter SIunits.SpecificHeatCapacity cpA=4200
    "specific heat capacity of fluid A";
  parameter SIunits.SpecificHeatCapacity cpB=4200
    "specific heat capacity of fluid B";
  parameter SIunits.SpecificHeatCapacity cpW=2000
    "specific heat capacity of the wall";
  parameter SIunits.CoefficientOfHeatTransfer gammaA=4000
    "heat transfer coefficient of fluid A";
  parameter SIunits.CoefficientOfHeatTransfer gammaB=10000
    "heat transfer coefficient of fluid B";
  parameter Real omega=0.1 "perimeter";
  final parameter Real l = L / (N - 1)
    "length of the each wall segment";
  discrete Real wA(start=1) "mass flow rate of fluid A";
  Real QA[N - 1]
    "heat flow rate of fluid A in the segments";
  Real QB[N - 1]
    "heat flow rate of fluid B in the segments";
//  Real TA_1 "temperature nodes on channel A";
//  Real TB_1 "temperature nodes on channel B";
  Real TA[N - 1] "temperature nodes on channel A";
  Real TB[N - 1] "temperature nodes on channel B";
  Real TW[N - 1] "temperatures on the wall segments";
  Real QtotA "total heat flow rate of fluid A";
  Real QtotB "total heat flow rate of fluid B";
  
  discrete Real TA_1(start=300);
  Real TB_1;
initial equation
  for i in 1:N-1 loop
    TA[i] = 300;
    TB[i] = 300;
    TW[i] = 300;
  end for;
equation
  //TA_1  = if time < 8 then 300 else 301;
  TB_1  = 310;
  //wA = if time < 15 then 1 else 1.1;
  
  
  rhoA * l * cpA * areaA * der(TA[1]) = wA * cpA * TA_1 - wA * cpA * TA[1] + QA[1];
  rhoB * l * cpB * areaB * der(TB[1]) = wB * cpB * TB_1 - wB * cpB * TB[1] - QB[1];
  QA[1] = (TW[1] - (TA_1 + TA[1]) / 2) * gammaA * omega * l;
  QB[1] = ((TB_1 + TB[1]) / 2 - TW[1]) * gammaB * omega * l;
  cpW / (N - 1) * der(TW[1]) = (-QA[1]) + QB[1];

  
  for i in 2:N-1 loop
    rhoA * l * cpA * areaA * der(TA[i ]) = wA * cpA * TA[i-1] - wA * cpA * TA[i ] + QA[i];
    rhoB * l * cpB * areaB * der(TB[i ]) = wB * cpB * TB[i-1] - wB * cpB * TB[i ] - QB[i];
    QA[i] = (TW[i] - (TA[i-1] + TA[i ]) / 2) * gammaA * omega * l;
    QB[i] = ((TB[i-1] + TB[i ]) / 2 - TW[i]) * gammaB * omega * l;
    cpW / (N - 1) * der(TW[i]) = (-QA[i]) + QB[i];
  end for;
  QtotA = sum(QA);
  QtotB = sum(QB);  
algorithm
  when time > 8 then
    TA_1 := 301;
  end when;
  when time > 15 then
    wA := 1.1;
    //wA = if time < 15 then 1 else 1.1;
  end when; 
  annotation(experiment(StopTime = 20, Tolerance = 1e-6));
end CocurrentHeatExchangerEquations;

