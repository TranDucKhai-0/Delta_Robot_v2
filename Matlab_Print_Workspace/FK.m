function pos = FK(theta)
% FK input RAD, output [x y z] mm hoặc [] nếu lỗi

p = delta_param();
rf = p.rf; re = p.re; a = p.a; k = p.k; phi = p.phi;

theta = theta(:).'; % 1x3
J = zeros(3,3);

for i = 1:3
    xJ = a + rf*cos(theta(i));
    zJ = rf*sin(theta(i));
    J(i,:) = [xJ - k, 0, zJ];
end

J_rot = zeros(3,3);
for i = 1:3
    J_rot(i,:) = passive_rotate(J(i,:), -phi(i));
end

J1 = J_rot(1,:); J2 = J_rot(2,:); J3 = J_rot(3,:);

A1 = 2*(J1(1) - J2(1));
B1 = 2*(J2(2));
C1 = 2*(J1(3) - J2(3));
D1 = J1(1)^2 - J2(1)^2 - J2(2)^2 + J1(3)^2 - J2(3)^2;

A2 = 2*(J1(1) - J3(1));
B2 = 2*(J3(2));
C2 = 2*(J1(3) - J3(3));
D2 = J1(1)^2 - J3(1)^2 - J3(2)^2 + J1(3)^2 - J3(3)^2;

denom = A1*B2 - A2*B1;
if abs(denom) < 1e-9
    pos = []; return;
end

A = (-D1*B2 + D2*B1) / denom;
C = (A1*D2 - A2*D1) / denom;
B = (-C1*B2 + C2*B1) / denom;
D = (A1*C2 - A2*C1) / denom;

bb = B^2 + D^2 + 1;
cc = 2*B*(A - J1(1)) + 2*C*D - 2*J1(3);
dd = (A - J1(1))^2 + C^2 + J1(3)^2 - re^2;

delta = cc*cc - 4*bb*dd;
if delta < 0
    pos = []; return;
end

z1 = (-cc + sqrt(delta)) / (2*bb);
z2 = (-cc - sqrt(delta)) / (2*bb);

z0 = min(z1, z2);
x0 = A + B*z0;
y0 = C + D*z0;

pos = [x0, y0, z0];
end

