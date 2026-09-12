/* Model for causalization testing =============================================
 * Description: model with a large algebraic loop of size k*N.
   iR[i] and iR[i+1] can be computed out of each other.
   uL[i] and uL[i+1] can be computed out of each other.
==============================================================================*/

model TestRL3
  constant Integer N=100;
  Real iL[N],iR[N],uL[N];
  parameter Real L=1,R=1,L1=1,U=1,R0=1;
equation
  for i in 1:N loop
    L*der(iL[i])=uL[i];
  end for;  
  for i in 1:N-1 loop
    iR[i]-iR[i+1]-iL[i]=0;
    uL[i]-uL[i+1]-R*iR[i+1]=0;
  end for;
  U-uL[1]-R*iR[1]=0;
  uL[N]-(iR[N]-iL[N])*R0=0;
end TestRL3;
