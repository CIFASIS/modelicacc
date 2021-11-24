model rlc_loop
    constant Integer N = 100000;
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
end rlc_loop;

/*
Representacion en modelicacc:

Vertices:
1:3 | IR2[i]
4:6 | IL[i]
7:9 | UC1[i]
10:12 | Ua[i]
13:15 | IR1[i]
16:18 | UC2[i]
19 | VR
20 | IR
21 | eq1
22 | eq2
23:25 | eq3
26:28 | eq4
29:31 | eq5
32:34 | eq6
35:36 | eq7
37:38 | eq8
39 | eq10
40 | eq11

Aristas:
1 | eq1 <-> IL[1]
2 | eq1 <-> Ua[1] 
3 | eq2 <-> IR2[N]
4 | eq2 <-> UC2[N]
5 | eq2 <-> IR
6:8 | eq3[i] <-> UC1[i]
9:11 | eq3[i] <-> Ua[i]
12:14 | eq3[i] <-> IR1[i]
15:17 | eq4[i] <-> IR2[i]
18:20 | eq4[i] <-> Ua[i]
21:23 | eq4[i] <-> UC2[i]
24:26 | eq5[i] <-> IR2[i]
27:29 | eq5[i] <-> IL[i]
30:32 | eq5[i] <-> IR1[i]  
33:35 | eq6[i] <-> UC1[i]
36:38 | eq6[i] <-> IR1[i]
39:40 | eq7[i] <-> IR2[i]
41:42 | eq7[i] <-> IL[i + 1]
43:44 | eq7[i] <-> UC2[i]
45:46 | eq8[i] <-> IL[i + 1] 
47:48 | eq8[i] <-> Ua[i + 1]
49:50 | eq8[i] <-> UC2[i]
51 | eq10 <-> VR
52 | eq10 <-> IR  
53 | eq11 <-> UC2[N]
54 | eq11 <-> VR
*/
