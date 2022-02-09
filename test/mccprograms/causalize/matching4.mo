model test4
  //Model with small algebraic loops and with connections between different indices in one direction
  constant Integer N= 10000;
  Real x1[N],a1[N],b1[N];
  Real x2[N], a2[N], b2[N];
  Real x3[N], a3[N], b3[N];
  Real x4[N], a4[N], b4[N];
  Real x5[N], a5[N], b5[N];
/*
  Real x6[N], a6[N], b6[N];
  Real x7[N], a7[N], b7[N];
  Real x8[N], a8[N], b8[N];
  Real x9[N], a9[N], b9[N];
*/
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

  a3[1]=2*x3[1]-b3[1];
  a3[1]=2*b3[1]-x3[1];
  der(x3[1])=1-a3[1];
  for i in 2:N loop
    a3[i]=2*x3[i]-b3[i];
    a3[i]=2*b3[i]-x3[i];
    der(x3[i])=a3[i-1]-a3[i];
  end for;  

  a4[1]=2*x4[1]-b4[1];
  a4[1]=2*b4[1]-x4[1];
  der(x4[1])=1-a4[1];
  for i in 2:N loop
    a4[i]=2*x4[i]-b4[i];
    a4[i]=2*b4[i]-x4[i];
    der(x4[i])=a4[i-1]-a4[i];
  end for;  

  a5[1]=2*x5[1]-b5[1];
  a5[1]=2*b5[1]-x5[1];
  der(x5[1])=1-a5[1];
  for i in 2:N loop
    a5[i]=2*x5[i]-b5[i];
    a5[i]=2*b5[i]-x5[i];
    der(x5[i])=a5[i-1]-a5[i];
  end for;  

/*
  a6[1]=2*x6[1]-b6[1];
  a6[1]=2*b6[1]-x6[1];
  der(x6[1])=1-a6[1];
  for i in 2:N loop
    a6[i]=2*x6[i]-b6[i];
    a6[i]=2*b6[i]-x6[i];
    der(x6[i])=a6[i-1]-a6[i];
  end for;  

  a7[1]=2*x7[1]-b7[1];
  a7[1]=2*b7[1]-x7[1];
  der(x7[1])=1-a7[1];
  for i in 2:N loop
    a7[i]=2*x7[i]-b7[i];
    a7[i]=2*b7[i]-x7[i];
    der(x7[i])=a7[i-1]-a7[i];
  end for;  

  a8[1]=2*x8[1]-b8[1];
  a8[1]=2*b8[1]-x8[1];
  der(x8[1])=1-a8[1];
  for i in 2:N loop
    a8[i]=2*x8[i]-b8[i];
    a8[i]=2*b8[i]-x8[i];
    der(x8[i])=a8[i-1]-a8[i];
  end for;  

  a9[1]=2*x9[1]-b9[1];
  a9[1]=2*b9[1]-x9[1];
  der(x9[1])=1-a9[1];
  for i in 2:N loop
    a9[i]=2*x9[i]-b9[i];
    a9[i]=2*b9[i]-x9[i];
    der(x9[i])=a9[i-1]-a9[i];
  end for;  
*/
end test4;
