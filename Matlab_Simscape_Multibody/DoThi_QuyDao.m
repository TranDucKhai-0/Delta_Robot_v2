
% --- Gán dữ liệu ---
% Dữ liệu IK (Mong muốn)
ik_x_ts = out.Eo_x_IK; 
ik_y_ts = out.Eo_y_IK; 
ik_z_ts = out.Eo_z_IK;

% Dữ liệu Simscape (Thực tế)
sim_x_ts = out.Eo_x_Simscape; 
sim_y_ts = out.Eo_y_Simscape; 
sim_z_ts = out.Eo_z_Simscape;

% Dữ liệu FK (Tính toán)
fk_x_ts = out.Eo_x_FK; 
fk_y_ts = out.Eo_y_FK; 
fk_z_ts = out.Eo_z_FK;

%% === TRÍCH XUẤT DỮ LIỆU RA MẢNG (ARRAY) ===
% Lấy thời gian chuẩn từ một biến bất kỳ (ví dụ sim_x_ts)
t = sim_x_ts.Time; 

% Ép về dạng vector để vẽ đồ thị
IK_X = squeeze(ik_x_ts.Data); 
IK_Y = squeeze(ik_y_ts.Data); 
IK_Z = squeeze(ik_z_ts.Data);

Sim_X = squeeze(sim_x_ts.Data); 
Sim_Y = squeeze(sim_y_ts.Data); 
Sim_Z = squeeze(sim_z_ts.Data);

FK_X = squeeze(fk_x_ts.Data); 
FK_Y = squeeze(fk_y_ts.Data); 
FK_Z = squeeze(fk_z_ts.Data);

% --- Tiếp tục các lệnh Figure phía sau của Đại Ca ---

%% ==========================================================
%% === PART 2: IK (DESIRED) vs. SIMSCAPE (ACTUAL) COMPARISON ===
%% ==========================================================

% --- FIGURE 1: X-axis Comparison (IK vs. Sim) ---
figure('Name', 'X-axis: IK vs. Simscape', 'Color', 'w');
plot(t, IK_X, 'r--', 'LineWidth', 2); hold on; 
plot(t, Sim_X, 'b-', 'LineWidth', 1.5);        
grid on; xlabel('Time (s)'); 
ylabel('X-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('X-axis Position Response');

% --- FIGURE 2: Y-axis Comparison (IK vs. Sim) ---
figure('Name', 'Y-axis: IK vs. Simscape', 'Color', 'w');
plot(t, IK_Y, 'r--', 'LineWidth', 2); hold on;
plot(t, Sim_Y, 'b-', 'LineWidth', 1.5);
grid on; xlabel('Time (s)'); 
ylabel('Y-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('Y-axis Position Response');

% --- FIGURE 3: Z-axis Comparison (IK vs. Sim) ---
figure('Name', 'Z-axis: IK vs. Simscape', 'Color', 'w');
plot(t, IK_Z, 'r--', 'LineWidth', 2); hold on;
plot(t, Sim_Z, 'b-', 'LineWidth', 1.5);
grid on; xlabel('Time (s)'); 
ylabel('Z-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('Z-axis Position Response');

%% ==========================================================
%% === PART 3: FK (CALCULATED) vs. SIMSCAPE (ACTUAL) COMPARISON ===
%% ==========================================================

% --- FIGURE 4: X-axis Comparison (FK vs. Sim) ---
figure('Name', 'X-axis: FK vs. Simscape', 'Color', 'w');
plot(t, FK_X, 'k--', 'LineWidth', 2); hold on; 
plot(t, Sim_X, 'g-', 'LineWidth', 1.5);        
grid on; xlabel('Time (s)'); 
ylabel('X-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('X-axis Position Response');

% --- FIGURE 5: Y-axis Comparison (FK vs. Sim) ---
figure('Name', 'Y-axis: FK vs. Simscape', 'Color', 'w');
plot(t, FK_Y, 'k--', 'LineWidth', 2); hold on;
plot(t, Sim_Y, 'g-', 'LineWidth', 1.5);
grid on; xlabel('Time (s)'); 
ylabel('Y-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('Y-axis Position Response');

% --- FIGURE 6: Z-axis Comparison (FK vs. Sim) ---
figure('Name', 'Z-axis: FK vs. Simscape', 'Color', 'w');
plot(t, FK_Z, 'k--', 'LineWidth', 2); hold on;
plot(t, Sim_Z, 'g-', 'LineWidth', 1.5);
grid on; xlabel('Time (s)'); 
ylabel('Z-Position (mm)');
legend('Analytical', 'Simulated', 'Location', 'best');
title('Z-axis Position Response');

%% ==========================================================
%% === PART 4: 3D TRAJECTORY VISUALIZATION ===
%% ==========================================================

% --- FIGURE 7: 3D Trajectory ---
figure('Name', '3D Trajectory Tracking', 'Color', 'w');
plot3(Sim_X, Sim_Y, Sim_Z, 'b-', 'LineWidth', 2); 
hold on; grid on; grid minor; axis equal;

% Start and End points
plot3(Sim_X(1), Sim_Y(1), Sim_Z(1), 'go', 'MarkerSize', 10, 'MarkerFaceColor', 'g');
plot3(Sim_X(end), Sim_Y(end), Sim_Z(end), 'rs', 'MarkerSize', 10, 'MarkerFaceColor', 'r');

% Workspace Surface (Decorative)
patch([-200 200 200 -200], [-200 -200 200 200], [0 0 0 0], ...
      'k', 'FaceAlpha', 0.05, 'EdgeColor', 'none');

xlabel('X-axis (mm)'); 
ylabel('Y-axis (mm)'); 
zlabel('Z-axis (mm)');
title('3D Workspace Trajectory Tracking');
legend('Actual Path', 'Start Point', 'End Point', 'Location', 'best');

zlim([-450 0]); 
view([45, 30]); box on;

% Auto-arrange windows
tilefigs;