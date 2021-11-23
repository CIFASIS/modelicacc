model test3
  //Model with small algebraic loops and without connections between different indices
  constant Integer N=1000000;
  Real x1[N],a1[N],b1[N], x2[N], a2[N], b2[N];
equation
  for i in 1:N loop
    a1[i]=2*x1[i]-b1[i];
    a1[i]=2*b1[i]-x1[i];
    der(x1[i])=1-a1[i];
  end for;  

  for i in 1:N loop
    a2[i]=2*x2[i]-b2[i];
    a2[i]=2*b2[i]-x2[i];
    der(x2[i])=1-a2[i];
  end for;  
end test3;
