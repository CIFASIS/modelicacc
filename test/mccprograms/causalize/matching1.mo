model test1
  //Model without algebraic loops and without connections between different indices
  constant Integer N=100000;
  Real x1[N],a1[N], x2[N], a2[N], x3[N], a3[N];
equation
  for i in 1:N loop
    a1[i]=x1[i];
    der(x1[i])=1-a1[i];
  end for;  

  for i in 1:N loop
    a2[i]=x2[i];
    der(x2[i])=1-a2[i];
  end for;  

  for i in 1:N loop
    a3[i]=x3[i];
    der(x3[i])=1-a3[i];
  end for;  
end test1;
