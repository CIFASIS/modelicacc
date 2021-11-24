model test5
  //Model with a large algebraic loop
  constant Integer N=1000;
  Real x1[N], a1[N];//, x2[N], a2[N];//, x3[N], a3[N], x4[N], a4[N];
equation
  a1[1]=x1[1]-a1[N];
  der(x1[1])=1-a1[1];
  for i in 2:N loop
    a1[i]=a1[i-1]-x1[i];
    der(x1[i])=a1[i];
  end for; 

  a2[1]=x2[1]-a2[N];
  der(x2[1])=1-a2[1];
  for i in 2:N loop
    a2[i]=a2[i-1]-x2[i];
    der(x2[i])=a2[i];
  end for; 

/*
  a3[1]=x[1]-a3[N];
  der(x3[1])=1-a3[1];
  for i in 2:N loop
    a3[i]=a3[i-1]-x3[i];
    der(x3[i])=a3[i];
  end for; 

  a4[1]=x4[1]-a4[N];
  der(x4[1])=1-a4[1];
  for i in 2:N loop
    a4[i]=a4[i-1]-x4[i];
    der(x4[i])=a4[i];
  end for; 
*/
end test5;
