#include "imgui.h"
#include "raylib.h"
#include "rlImGui.h"
#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <string>
#include <vector>

// ALL UNITS ARE cm
// 00 is the center of the robot at the point of the ground

class Cone {
public:
  glm::vec2 cameraPos = {0, 15.782};
  int resolution = 200;
  int range = 350;

  glm::vec2 origin = {45 * 16, 26 * 32 - 200};
  glm::vec2 factor = {20, -20};

  float approximateConeRadius = 1.5;

  std::vector<glm::vec2> targetPoints;

  std::vector<glm::vec2> conePoints;

  glm::vec2 coneStartPos = {0, 17};

  float halfFov = 30;

  Cone() {
    for (int i = 0; i < resolution; i++) {
      targetPoints.push_back({(float)i * (float)range / (float)resolution, 0});
    }
  }

  void draw() {
    glm::vec2 drawCameraPos = getPointFromOrigin(cameraPos);

    DrawLine(origin.x, 0, origin.x, GetScreenHeight(), {0, 0, 0, 255});
    DrawLine(0, origin.y, GetScreenWidth(), origin.y, {0, 0, 0, 255});

    DrawLine(getPointFromOrigin(cameraPos).x, getPointFromOrigin(cameraPos).y, (cameraPos.x + 500 * std::tan(halfFov / 180 * 3.14159)) * factor.x + origin.x, (cameraPos.y + 500) * factor.y + origin.y, {0, 0, 0, 255});
    DrawLine(getPointFromOrigin(cameraPos).x, getPointFromOrigin(cameraPos).y, (cameraPos.x - 500 * std::tan(halfFov / 180 * 3.14159)) * factor.x + origin.x, (cameraPos.y + 500) * factor.y + origin.y, {0, 0, 0, 255});

    /* DrawLine(drawCameraPos.x, drawCameraPos.y, drawCameraPos.x, drawCameraPos.y - 2.7 * factor.y, {255, 0, 0, 255}); */
    /* DrawLine(drawCameraPos.x + 4 * factor.x, drawCameraPos.y - 2.7, drawCameraPos.x, drawCameraPos.y - 2.7 * factor.y, {255, 0, 0, 255}); */

    drawCircleFromOrigin(cameraPos);

    for (int i = 0; i < resolution; i++) {
      drawCircleFromOrigin(targetPoints[i], 5, {0, 0, 255, 255});
    }

    drawCone();
  }

  void update() {
    /* approximateConeRadius = std::tan(halfFov / 180 * 3.14159265) * (coneStartPos.y - cameraPos.y); */
    targetPoints = {};
    for (int i = 0; i < resolution; i++) {
      targetPoints.push_back({(float)i * (float)range / (float)resolution, 0});
    }
    generateCone();
  }

private:
  void drawCircleFromOrigin(glm::vec2 pos, int r = 5, Color col = {255, 0, 0, 255}) {
    DrawCircle(pos.x * factor.x + origin.x, origin.y + pos.y * factor.y, r, col);
  }

  glm::vec2 getPointFromOrigin(glm::vec2 pos) {
    return {pos.x * factor.x + origin.x, origin.y + pos.y * factor.y};
  }

  void generateCone() {
    conePoints = {coneStartPos};
    for (int i = 1; i < resolution; i++) {
      glm::vec2 pointToCamera = cameraPos - conePoints[i - 1];
      glm::vec2 pointToTarget = targetPoints[i] - conePoints[i - 1];

      float angle = (atan2(pointToTarget.y, pointToTarget.x) - atan2(pointToCamera.y, pointToCamera.x)) / 2;
      angle = 1.5707963268 - angle;

      glm::vec2 pointDiff = {pointToTarget.x * glm::cos(angle) - pointToTarget.y * sin(angle), pointToTarget.x * glm::sin(angle) + pointToTarget.y * cos(angle)};
      pointDiff = glm::normalize(pointDiff);
      pointDiff *= approximateConeRadius / resolution;

      conePoints.push_back(conePoints[i - 1] + pointDiff);
    }
  }

  void drawCone() {
    for (int i = 1; i < resolution; i++) {
      /* drawCircleFromOrigin(conePoints[i], 1, {255, 0, 0, 255}); */
      DrawLine(conePoints[i - 1].x * factor.x + origin.x, conePoints[i - 1].y * factor.y + origin.y, conePoints[i].x * factor.x + origin.x, conePoints[i].y * factor.y + origin.y, {255, 0, 0, 255});
      DrawLine(-conePoints[i - 1].x * factor.x + origin.x, conePoints[i - 1].y * factor.y + origin.y, -conePoints[i].x * factor.x + origin.x, conePoints[i].y * factor.y + origin.y, {255, 0, 0, 255});

      /* if (i % 10 == 0) { */
      /*   DrawLine(conePoints[i].x * factor.x + origin.x, conePoints[i].y * factor.y + origin.y, getPointFromOrigin(targetPoints[i]).x, getPointFromOrigin(targetPoints[i]).y, {255, 0, 0, 255}); */
      /* } */
      /* drawCircleFromOrigin({-conePoints[i].x, conePoints[i].y}, 1, {255, 0, 0, 255}); */
    }
  }
};

int saveCone(Cone cone) {
  uint32_t slices = 50;
  uint32_t triangleCount = (cone.conePoints.size() - 1) * slices * 2 + slices; // +slices for cap

  std::ofstream file("cone.stl", std::ios::binary);
  if (!file.is_open())
    return 1;

  char header[80] = {};
  file.write(header, 80);
  file.write(reinterpret_cast<char *>(&triangleCount), 4);

  struct Triangle {
    float normal[3];
    float vertex1[3];
    float vertex2[3];
    float vertex3[3];
    uint16_t attr = 0;
  } __attribute__((packed));

  // side faces
  for (int s = 0; s < slices; s++) {
    float angle1 = (2 * M_PI * s) / slices;
    float angle2 = (2 * M_PI * (s + 1)) / slices;

    for (int i = 0; i < cone.conePoints.size() - 1; i++) {
      auto &p1 = cone.conePoints[i];
      auto &p2 = cone.conePoints[i + 1];

      float v1[3] = {p1.x * cos(angle1), p1.y, p1.x * sin(angle1)};
      float v2[3] = {p1.x * cos(angle2), p1.y, p1.x * sin(angle2)};
      float v3[3] = {p2.x * cos(angle1), p2.y, p2.x * sin(angle1)};
      float v4[3] = {p2.x * cos(angle2), p2.y, p2.x * sin(angle2)};

      Triangle t1, t2;

      // flipped winding order for inverted normals
      memcpy(t1.vertex1, v1, 12);
      memcpy(t1.vertex2, v3, 12);
      memcpy(t1.vertex3, v2, 12);

      memcpy(t2.vertex1, v2, 12);
      memcpy(t2.vertex2, v3, 12);
      memcpy(t2.vertex3, v4, 12);

      auto calcNormal = [](float *n, float *a, float *b, float *c) {
        float u[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
        float v[3] = {c[0] - a[0], c[1] - a[1], c[2] - a[2]};
        n[0] = u[1] * v[2] - u[2] * v[1];
        n[1] = u[2] * v[0] - u[0] * v[2];
        n[2] = u[0] * v[1] - u[1] * v[0];
        float len = sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]);
        if (len > 0) {
          n[0] /= len;
          n[1] /= len;
          n[2] /= len;
        }
      };

      calcNormal(t1.normal, t1.vertex1, t1.vertex2, t1.vertex3);
      calcNormal(t2.normal, t2.vertex1, t2.vertex2, t2.vertex3);

      file.write(reinterpret_cast<char *>(&t1), 50);
      file.write(reinterpret_cast<char *>(&t2), 50);
    }
  }

  // top cap
  auto &topPoint = cone.conePoints.back();
  float center[3] = {0, topPoint.y, 0};

  for (int s = 0; s < slices; s++) {
    float angle1 = (2 * M_PI * s) / slices;
    float angle2 = (2 * M_PI * (s + 1)) / slices;

    Triangle cap;
    memcpy(cap.vertex1, center, 12);
    cap.vertex2[0] = topPoint.x * cos(angle2);
    cap.vertex2[1] = topPoint.y;
    cap.vertex2[2] = topPoint.x * sin(angle2);
    cap.vertex3[0] = topPoint.x * cos(angle1);
    cap.vertex3[1] = topPoint.y;
    cap.vertex3[2] = topPoint.x * sin(angle1);

    // normal points up
    cap.normal[0] = 0;
    cap.normal[1] = 1;
    cap.normal[2] = 0;

    file.write(reinterpret_cast<char *>(&cap), 50);
  }

  file.close();
  return 0;
}

void update(Cone &cone) {
  cone.update();
}

void draw(Cone &cone) {
  cone.draw();
}

int main() {
  /* const int screenWidth = 32 * 45; */
  const int screenWidth = 2560;
  const int screenHeight = 1440;

  /* const int screenHeight = 32 * 26; */

  rlImGuiSetup(true); // true for dark theme

  InitWindow(screenWidth, screenHeight, "Game");
  SetTargetFPS(60);

  Cone cone = Cone();

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    update(cone);

    rlImGuiBegin();

    ImGui::Begin("Parameters. All lengths are in cm");
    ImGui::SliderInt("Resolution", &cone.resolution, 10, 5000);
    ImGui::SliderFloat("Camera height above ground", &cone.cameraPos.y, 0.0f, 50.0f);
    ImGui::DragInt("Range of view", &cone.range, 1, 0, 100000000);
    ImGui::SliderFloat("Approximate cone radius", &cone.approximateConeRadius, 0.5f, 10.0f);
    ImGui::DragFloat("Cone starting y", &cone.coneStartPos.y, 0.1f, 0.0f, 20.0f);
    ImGui::DragFloat("Camera half FOV", &cone.halfFov, 0.1f, 0.0f, 180.f);
    ImGui::Text("Cone's top point: %.2f", cone.conePoints[cone.resolution - 1].y);
    ImGui::Text("True cone radius: %.2f", cone.conePoints[cone.resolution - 1].x);
    ImGui::Text("Approximate cone radius: %.2f", cone.approximateConeRadius);

    ImGui::End();

    /* static bool show_demo = true; */
    /* ImGui::ShowDemoWindow(&show_demo); */

    rlImGuiEnd();

    draw(cone);

    EndDrawing();
  }
  saveCone(cone);

  rlImGuiShutdown();
  CloseWindow();
  return 0;
}
