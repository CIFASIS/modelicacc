model RLC
  constant Integer N = 1000;
  parameter Real R1=1,R2=1,L=1,C1=1,C2=1,Vs=1,R=1;
  Real IR2[N], IL[N], UC1[N], Ua[N], IR1[N], UC2[N];
  Real VR, IR;
equation
  L*der(IL[1]) = Vs - Ua[1];
  C2*der(UC2[N]) = IR2[N] - IR;
  for i in 1:N loop
    IR1[i] = (Ua[i] - UC1[i])/R1;
    IR2[i] = (Ua[i] - UC2[i])/R2;
    IL[i] = IR1[i] + IR2[i];
    C1*der(UC1[i]) = IR1[i];
  end for;
  for i in 1:N-1 loop
    C2*der(UC2[i]) = IR2[i] - IL[i+1];
    L*der(IL[i+1]) = UC2[i]-Ua[i+1];
  end for;
  VR = R*IR;
  UC2[N] = VR;
end RLC;
