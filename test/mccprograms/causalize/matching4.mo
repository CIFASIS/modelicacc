model test4
  //Model with small algebraic loops and with connections between different indices in one direction
  constant Integer N= 100;
  Real x1[N],a1[N],b1[N], x2[N], a2[N], b2[N];
equation
  a1[1]=2*x1[1]-b1[1];
  a1[1]=2*b1[1]-x1[1];
  der(x1[1])=1-a1[1];
  for i in 2:N loop
    a1[i]=2*x1[i]-b1[i];
    a1[i]=2*b1[i]-x1[i];
    der(x1[i])=a1[i-1]-a1[i];
  end for;  

  a2[1]=2*x2[1]-b2[1];
  a2[1]=2*b2[1]-x2[1];
  der(x2[1])=1-a2[1];
  for i in 2:N loop
    a2[i]=2*x2[i]-b2[i];
    a2[i]=2*b2[i]-x2[i];
    der(x2[i])=a2[i-1]-a2[i];
  end for;  
end test4;
