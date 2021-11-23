model test6
  //Model with possibly wrong initial matchingTest
  constant Integer N=10;
  Real x1,a1[N], x2, a2[N];
equation 
  der(x1)=1-a1[1];
  a1[N]=x1;
  for i in 2:N loop
    a1[i-1]=a1[i];
  end for;

  der(x2)=1-a2[1];
  a2[N]=x2;
  for i in 2:N loop
    a2[i-1]=a2[i];
  end for;
end test6;
