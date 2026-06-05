/* Model for causalization testing =============================================
 * Description: simple scalar model without additional properties, based on
   the model of TestRL2.mo with N = 1.
==============================================================================*/

model scalar
  Real iL,iR,uL;
  parameter Real L=1,R=1,I=1,R0=1;
equation
  L*der(iL)=uL;
  iR-iL+I=0;
  uL+(R+R0)*iR=0;
end scalar;
