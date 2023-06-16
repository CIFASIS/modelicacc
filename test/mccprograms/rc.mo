model rc
  constant Integer n=10000;
  parameter Real RA=1,RB=1,C=1,i0=1;
  Real vA[n],vB[n],vC[n],iR[n];
equation
  iR[1]=i0; // Eq.(1)
  for i in 1:n loop
    vA[i]=RA*iR[i]; //Eq.(2)
   vB[i]=RB*iR[i]; //Eq.(3)
  end for;
  for i in 2:n loop
    vA[i]+vB[i]=vC[i-1]-vC[i]; //Eq.(4)
  end for;
  for i in 1:n-1 loop
    C*der(vC[i])=iR[i]-iR[i+1]; //Eq.(5)
  end for;
  C*der(vC[n])=iR[n]; //Eq.(6)
end rc;
