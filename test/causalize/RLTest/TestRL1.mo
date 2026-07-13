/* Model for causalization testing =============================================
 * Description: model with N algebraic loops.
   Ua[i] and Uc[i] must be simultaneously solved.
==============================================================================*/

model TestRL1
  constant Integer N = 100;
  parameter Real U0 = 1.0;
  Real iL[N], Ua[N], Uc[N];
  parameter Real Ra = 1, Rb = 1, Rc = 1, L = 1;
equation
  L*der(iL[1]) = U0 - Ua[1];
  for i in 2:N loop
    L*der(iL[i]) = Uc[i-1] - Ua[i];
  end for;
  for i in 1:N-1 loop
    Ua[i] = Rb*iL[i+1] + Uc[i]*Rb/Rc;
  end for;
  for i in 1:N-1 loop
    iL[i] = Ua[i]/Ra + Uc[i]/Rc + iL[i+1];
  end for;
  iL[N] = Ua[N]/Ra + Uc[N]/Rc;
  Ua[N] = Uc[N]*Rb/Rc;
end TestRL1;

//Algebraic Loops:
//der(iL[1])
//der(iL[1]) = U0-Ua[1]
//
//der(iL[i])
//L*der(iL[i]) = Uc[i-1]-Ua[i]
//
//Ua[i]
//Uc[i]
//Ua[i] = Rb*iL[i+1]+Uc[i]*Rb/Rc
//iL[i] = Ua[i]/Ra+Uc[i]/Rc+iL[i+1]
//
//Uc[N]
//Ua[N]
//iL[N] = Ua[N]/Ra+Uc[N]/Rc
//Ua[N] = Uc[N]*Rb/Rc
