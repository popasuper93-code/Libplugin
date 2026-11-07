// CHAD Premium — Dear ImGui UI layer (header-only, C++17, ImGui 1.90.x / 1.91.x)
// Usage: chad::LoadFonts("fonts") before the font atlas is built, chad::Render() between NewFrame() and Render().
// Fonts (optional, Cyrillic ranges are loaded): fonts/Inter-Medium.ttf, Inter-ExtraBold.ttf, JetBrainsMono-Regular.ttf
// Save this file as UTF-8 (MSVC: /utf-8).
#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <string>
#include <unordered_map>

namespace chad {

struct Bind { const char* key; int mode; };

struct Config {
    bool menuOpen = true;
    bool sul = true, stn = true, wod = false, met = false, mag = false, avd = false;
    float farmDist = 120, scanSpeed = 6;
    bool aim = true, silent = false, drawFov = true, visCheck = true, silentVis = false;
    float fov = 90, smooth = 35, aimDist = 250, hitChance = 85, silentDist = 300;
    int bone = 0;
    bool esp = true, box = true, name = true, hp = true, dist = true, weapon = true, skel = true, chams = true, loot = true;
    int boxStyle = 1, espCol = 0;
    float espDist = 300, lineTh = 1, textSz = 11, chamsOp = 55, lootDist = 120;
    bool freeCam = false, night = false, noGrass = false;
    float freeCamSpeed = 5, tod = 12;
    bool sounds = true;
    int lang = 0, accent = 0;
    float menuOpacity = 92, menuScale = 100, hudScale = 100;
    Bind bAim{"MB5", 0}, bSil{"ALT", 1}, bCam{"F3", 1};
};

inline Config cfg;
inline bool injected = false;
inline float ping = 24.f;
inline void (*sound)(float) = nullptr;

constexpr ImU32 TX = IM_COL32(224, 224, 224, 255), T2 = IM_COL32(155, 155, 165, 255), T3 = IM_COL32(94, 94, 104, 255);
constexpr ImU32 LN = IM_COL32(34, 34, 40, 255), LN2 = IM_COL32(47, 47, 55, 255), P1 = IM_COL32(17, 17, 20, 255);
constexpr ImU32 OK = IM_COL32(61, 220, 132, 255), ER = IM_COL32(229, 72, 77, 255), WH = IM_COL32(255, 255, 255, 255);
constexpr ImU32 PAL[8] = {IM_COL32(138, 43, 226, 255), IM_COL32(229, 50, 74, 255), IM_COL32(43, 217, 107, 255), IM_COL32(226, 183, 43, 255),
                          IM_COL32(43, 127, 226, 255), IM_COL32(224, 224, 224, 255), IM_COL32(255, 122, 26, 255), IM_COL32(26, 209, 209, 255)};
constexpr ImU32 ACC[3] = {IM_COL32(138, 43, 226, 255), IM_COL32(43, 127, 226, 255), IM_COL32(229, 50, 74, 255)};

inline ImU32 C(int r, int g, int b, int a = 255) { return IM_COL32(r, g, b, a); }
inline ImU32 Acc() { return ACC[cfg.accent]; }
inline float Sc() { return cfg.menuScale / 100.f; }
inline float px(float v) { return v * Sc(); }
inline void Snd(float f) { if (cfg.sounds && sound) sound(f); }

inline ImFont *fR = nullptr, *fB = nullptr, *fM = nullptr;

inline ImFont* LoadFont(const char* dir, const char* file, float size) {
    std::string p = std::string(dir) + "/" + file;
    FILE* f = std::fopen(p.c_str(), "rb");
    if (!f) return nullptr;
    std::fclose(f);
    ImGuiIO& io = ImGui::GetIO();
    return io.Fonts->AddFontFromFileTTF(p.c_str(), size, nullptr, io.Fonts->GetGlyphRangesCyrillic());
}

inline void LoadFonts(const char* dir = "fonts") {
    fR = LoadFont(dir, "Inter-Medium.ttf", 14);
    fB = LoadFont(dir, "Inter-ExtraBold.ttf", 22);
    fM = LoadFont(dir, "JetBrainsMono-Regular.ttf", 13);
    if (!fR) fR = ImGui::GetIO().Fonts->AddFontDefault();
    if (!fB) fB = fR;
    if (!fM) fM = fR;
    ImGui::GetIO().FontDefault = fR;
}

inline const char* L(const char* k) {
    static const std::unordered_map<std::string, const char*> ru = {
        {"Farm", "Фарм"}, {"Aimbot", "Аимбот"}, {"Silent Aim", "Сайлент аим"}, {"Visuals", "Визуалы"}, {"World", "Мир"}, {"Settings", "Настройки"},
        {"Main", "Основное"}, {"Render", "Рендер"}, {"Config", "Конфиг"}, {"Players", "Игроки"}, {"Chams", "Чамсы"}, {"Resources", "Ресурсы"},
        {"Parameters", "Параметры"}, {"Session", "Сессия"}, {"General", "Общее"}, {"Target", "Цель"}, {"Filters", "Фильтры"}, {"Player ESP", "ESP игроков"},
        {"Preview", "Превью"}, {"Color", "Цвет"}, {"Camera", "Камера"}, {"Environment", "Окружение"}, {"Interface", "Интерфейс"}, {"Accent", "Акцент"},
        {"Loot", "Лут"}, {"Sulfur", "Сера"}, {"Stone", "Камень"}, {"Wood", "Дерево"}, {"Metal", "Металл"}, {"Avoid players 60 m", "Обход игроков 60 м"},
        {"Farm distance", "Дистанция фарма"}, {"Scan speed", "Скорость скана"}, {"Nodes", "Точки"}, {"Yield / h", "Добыча / ч"}, {"Uptime", "Время работы"},
        {"Enable", "Включить"}, {"Smoothness", "Плавность"}, {"Distance", "Дистанция"}, {"Hitbox", "Кость"}, {"Head", "Голова"}, {"Neck", "Шея"},
        {"Chest", "Грудь"}, {"Draw FOV", "Рисовать FOV"}, {"Visible check", "Проверка видимости"}, {"Hit chance", "Шанс попадания"},
        {"Visible only", "Только видимые"}, {"Enable ESP", "Включить ESP"}, {"Box", "Рамка"}, {"Box style", "Стиль рамки"}, {"Full", "Полная"},
        {"Corners", "Углы"}, {"Name", "Имя"}, {"Health bar", "Полоса HP"}, {"Weapon", "Оружие"}, {"Skeleton", "Скелет"}, {"Max distance", "Макс. дистанция"},
        {"Line thickness", "Толщина линий"}, {"Text size", "Размер текста"}, {"Enable chams", "Включить чамсы"}, {"Opacity", "Прозрачность"},
        {"Loot ESP", "ESP лута"}, {"Loot distance", "Дистанция лута"}, {"Free Cam", "Свободная камера"}, {"Free cam speed", "Скорость камеры"},
        {"Bright Night", "Яркая ночь"}, {"No Grass", "Без травы"}, {"Time of day", "Время суток"}, {"Sounds", "Звуки"}, {"Menu opacity", "Прозрачность меню"},
        {"Menu size", "Размер меню"}, {"HUD scale", "Масштаб HUD"}, {"Language", "Язык"}, {"Purple", "Фиолетовый"}, {"Blue", "Синий"}, {"Red", "Красный"},
        {"Hold", "Удерж."}, {"Toggle", "Перекл."}, {"Always", "Всегда"}, {"Search element", "Поиск"}, {"active", "вкл"}, {"idle", "выкл"}};
    if (cfg.lang == 1) {
        auto i = ru.find(k);
        if (i != ru.end()) return i->second;
    }
    return k;
}

inline char search[64] = "";
inline ImVec2 gClipA, gClipB;
inline float gColX = 0;

inline bool Match(const char* id) {
    if (!search[0]) return true;
    std::string a = L(id), b = search;
    auto lw = [](std::string& s) { for (auto& c : s) c = (char)std::tolower((unsigned char)c); };
    lw(a); lw(b);
    return a.find(b) != std::string::npos;
}

inline void Spaced(ImDrawList* d, ImFont* f, float sz, ImVec2 p, ImU32 c, const char* s, float sp) {
    for (; *s; ++s) {
        char b[2] = {*s, 0};
        d->AddText(f, sz, p, c, b);
        p.x += f->CalcTextSizeA(sz, FLT_MAX, 0, b).x + sp;
    }
}

inline bool Check(const char* id, bool* v, Bind* kb = nullptr) {
    if (!Match(id)) return false;
    ImDrawList* d = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x, h = px(24), kw = 0, b12 = px(12);
    char kt[32] = "";
    if (kb) {
        static const char* md[] = {"Hold", "Toggle", "Always"};
        std::snprintf(kt, sizeof kt, "%s · %s", kb->key, L(md[kb->mode]));
        kw = fM->CalcTextSizeA(px(11), FLT_MAX, 0, kt).x + px(14);
    }
    ImGui::PushID(id);
    bool hit = ImGui::InvisibleButton("##c", ImVec2(w - kw, h));
    if (hit) { *v = !*v; Snd(*v ? 1800.f : 1100.f); }
    if (kb) {
        ImGui::SameLine(0, 0);
        if (ImGui::InvisibleButton("##k", ImVec2(kw, h))) { kb->mode = (kb->mode + 1) % 3; Snd(1400.f); }
        ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
        bool hv = ImGui::IsItemHovered();
        a.y += px(4); b.y -= px(4);
        d->AddRect(a, b, hv ? LN2 : LN);
        d->AddText(fM, px(11), ImVec2(a.x + px(7), a.y + (b.y - a.y - px(11)) * .5f), hv ? TX : T3, kt);
    }
    ImGui::PopID();
    bool on = *v;
    ImVec2 a(p.x, p.y + (h - b12) * .5f), b = a + ImVec2(b12, b12);
    d->AddRectFilled(a, b, on ? Acc() : C(10, 10, 12));
    d->AddRect(a, b, on ? Acc() : C(59, 59, 69));
    if (on) {
        float t = .5f + .5f * std::sin((float)ImGui::GetTime() * 2.4f);
        d->AddRect(a - ImVec2(2, 2), b + ImVec2(2, 2), (Acc() & 0x00FFFFFFu) | ((ImU32)(60 * t) << 24));
        d->AddLine(a + ImVec2(b12 * .25f, b12 * .52f), a + ImVec2(b12 * .43f, b12 * .75f), WH, px(1.6f));
        d->AddLine(a + ImVec2(b12 * .43f, b12 * .75f), a + ImVec2(b12 * .78f, b12 * .25f), WH, px(1.6f));
    }
    d->AddText(ImVec2(p.x + px(21), p.y + (h - ImGui::GetFontSize()) * .5f), on ? TX : T2, L(id));
    return hit;
}

inline bool Slide(const char* id, float* v, float lo, float hi, const char* unit = "") {
    if (!Match(id)) return false;
    ImDrawList* d = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x, fs = px(11);
    char b[32];
    std::snprintf(b, sizeof b, "%d%s", (int)std::lround(*v), unit);
    ImVec2 ts = fM->CalcTextSizeA(fs, FLT_MAX, 0, b);
    d->AddText(ImVec2(p.x, p.y + px(6)), T2, L(id));
    d->AddText(fM, fs, ImVec2(p.x + w - ts.x, p.y + px(7)), TX, b);
    ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + px(24)));
    ImGui::PushID(id);
    ImGui::InvisibleButton("##s", ImVec2(w, px(12)));
    bool ch = false;
    if (ImGui::IsItemActive()) {
        float t = std::clamp((ImGui::GetIO().MousePos.x - p.x) / w, 0.f, 1.f);
        float nv = std::round(lo + (hi - lo) * t);
        if (nv != *v) { *v = nv; ch = true; }
    }
    ImGui::PopID();
    float t = (*v - lo) / (hi - lo), y = p.y + px(28);
    ImVec2 a(p.x, y), e(p.x + w, y + px(4));
    d->AddRectFilled(a, e, C(29, 29, 34));
    d->AddRectFilled(a, ImVec2(a.x + w * t, e.y), Acc());
    d->AddRect(a, e, C(0, 0, 0));
    float tx = a.x + w * t;
    d->AddRectFilled(ImVec2(tx - px(2), y - px(3)), ImVec2(tx + px(2), y + px(7)), TX);
    ImGui::Dummy(ImVec2(1, px(4)));
    return ch;
}

inline bool Seg(const char* id, int* sel, const char* const* it, int n, const ImU32* sw = nullptr, bool label = true) {
    if (!Match(id)) return false;
    ImDrawList* d = ImGui::GetWindowDrawList();
    if (label) {
        ImVec2 p = ImGui::GetCursorScreenPos();
        d->AddText(ImVec2(p.x, p.y + px(6)), T2, L(id));
        ImGui::Dummy(ImVec2(1, px(26)));
    }
    float g = px(3), w = ImGui::GetContentRegionAvail().x, bw = (w - g * (n - 1)) / n, h = px(26);
    bool ch = false;
    ImGui::PushID(id);
    for (int i = 0; i < n; i++) {
        if (i) ImGui::SameLine(0, g);
        ImGui::PushID(i);
        if (ImGui::InvisibleButton("##b", ImVec2(bw, h)) && *sel != i) { *sel = i; ch = true; Snd(1600.f); }
        ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
        bool on = *sel == i;
        d->AddRectFilled(a, b, on ? C(24, 24, 28) : C(13, 13, 16));
        d->AddRect(a, b, on ? LN2 : LN);
        if (on) d->AddRectFilled(ImVec2(a.x, b.y - 1), b, Acc());
        const char* s = L(it[i]);
        ImVec2 ts = ImGui::CalcTextSize(s);
        float x0 = a.x + (bw - ts.x) * .5f;
        if (sw) {
            x0 += px(6);
            d->AddRectFilled(ImVec2(x0 - px(13), a.y + (h - px(7)) * .5f), ImVec2(x0 - px(6), a.y + (h + px(7)) * .5f), sw[i]);
        }
        d->AddText(ImVec2(x0, a.y + (h - ts.y) * .5f), on ? WH : T2, s);
        ImGui::PopID();
    }
    ImGui::PopID();
    return ch;
}

inline void Swatch(int* sel) {
    ImDrawList* d = ImGui::GetWindowDrawList();
    ImGui::Dummy(ImVec2(1, px(4)));
    for (int i = 0; i < 8; i++) {
        if (i) ImGui::SameLine(0, px(9));
        ImGui::PushID(i);
        if (ImGui::InvisibleButton("##w", ImVec2(px(22), px(22)))) { *sel = i; Snd(1600.f); }
        ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
        d->AddRectFilled(a, b, PAL[i]);
        d->AddRect(a, b, C(0, 0, 0));
        if (*sel == i) d->AddRect(a - ImVec2(px(3), px(3)), b + ImVec2(px(3), px(3)), TX);
        ImGui::PopID();
    }
    ImGui::Dummy(ImVec2(1, px(10)));
}

inline void Row(const char* a, const char* b) {
    ImDrawList* d = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float w = ImGui::GetContentRegionAvail().x, fs = px(11);
    ImVec2 ts = fM->CalcTextSizeA(fs, FLT_MAX, 0, b);
    d->AddText(fM, fs, ImVec2(p.x, p.y + px(5)), T3, L(a));
    d->AddText(fM, fs, ImVec2(p.x + w - ts.x, p.y + px(5)), TX, b);
    d->AddLine(ImVec2(p.x, p.y + px(23)), ImVec2(p.x + w, p.y + px(23)), LN);
    ImGui::Dummy(ImVec2(w, px(24)));
}

inline void Card(const char* id, bool* v, ImVec2 pos, ImVec2 sz) {
    ImDrawList* d = ImGui::GetWindowDrawList();
    ImGui::SetCursorScreenPos(pos);
    ImGui::PushID(id);
    if (ImGui::InvisibleButton("##f", sz)) { *v = !*v; Snd(*v ? 1800.f : 1100.f); }
    ImGui::PopID();
    ImVec2 a = pos, b = pos + sz;
    bool on = *v;
    d->AddRectFilled(a, b, on ? C(14, 18, 16) : C(13, 13, 16));
    d->AddRect(a, b, on ? C(47, 58, 51) : LN);
    d->AddRectFilled(a, ImVec2(a.x + px(3), b.y), on ? OK : C(92, 43, 50));
    d->AddText(ImVec2(a.x + px(15), a.y + px(9)), TX, L(id));
    d->AddText(fM, px(10), ImVec2(a.x + px(15), b.y - px(19)), on ? OK : C(138, 74, 82), L(on ? "active" : "idle"));
}

struct BoxInfo { ImVec2 pos; const char* t; };
inline BoxInfo gBox[4];
inline int gN = 0;

inline void BeginBox(const char* t, float w) {
    ImGui::SetCursorPosX(gColX);
    gBox[gN++] = {ImGui::GetCursorScreenPos(), t};
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(12), px(16)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, P1);
    ImGui::PushStyleColor(ImGuiCol_Border, LN);
    ImGui::BeginChild(t, ImVec2(w, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
}

inline void EndBox() {
    ImGui::Dummy(ImVec2(1, px(7)));
    ImDrawList* d = ImGui::GetWindowDrawList();
    BoxInfo bi = gBox[--gN];
    const char* t = L(bi.t);
    ImVec2 ts = ImGui::CalcTextSize(t), wp = ImGui::GetWindowPos();
    d->PushClipRect(gClipA, gClipB, false);
    float x = wp.x + px(9);
    d->AddRectFilled(ImVec2(x, wp.y - 1), ImVec2(x + ts.x + px(12), wp.y + ts.y * .5f), P1);
    d->AddText(ImVec2(x + px(6), wp.y - ts.y * .5f), T2, t);
    d->PopClipRect();
    ImGui::EndChild();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);
}

template <class F> inline void Box(const char* t, float w, F fn) {
    BeginBox(t, w);
    fn();
    EndBox();
    ImGui::Dummy(ImVec2(1, px(14)));
}

template <class A, class B> inline void Cols(A a, B b) {
    float x0 = ImGui::GetCursorPosX(), W = ImGui::GetContentRegionAvail().x, g = px(14), wl = (W - g) * .535f, wr = W - g - wl;
    ImVec2 s = ImGui::GetCursorPos();
    gColX = x0;
    a(wl);
    float yl = ImGui::GetCursorPosY();
    ImGui::SetCursorPos(ImVec2(x0 + wl + g, s.y));
    gColX = x0 + wl + g;
    b(wr);
    float yr = ImGui::GetCursorPosY();
    ImGui::SetCursorPos(ImVec2(x0, std::max(yl, yr)));
    ImGui::Dummy(ImVec2(1, 1));
    gColX = x0;
}

inline void Preview() {
    ImDrawList* d = ImGui::GetWindowDrawList();
    ImVec2 a = ImGui::GetCursorScreenPos();
    float W = ImGui::GetContentRegionAvail().x, H = px(330);
    ImGui::Dummy(ImVec2(W, H));
    for (float y = a.y; y < a.y + H; y += px(12))
        for (float x = a.x; x < a.x + W; x += px(12)) d->AddRectFilled(ImVec2(x, y), ImVec2(x + 1, y + 1), C(255, 255, 255, 14));
    float k = H / 380.f;
    ImVec2 o(a.x + W * .5f - 90 * k, a.y);
    auto P = [&](float x, float y) { return ImVec2(o.x + (10 + x) * k, o.y + (40 + y) * k); };
    ImU32 esp = PAL[cfg.espCol], fill = C(42, 42, 49), edge = C(60, 60, 69);
    if (cfg.chams) {
        float t = cfg.chamsOp / 100.f;
        ImVec4 e = ImGui::ColorConvertU32ToFloat4(esp);
        fill = ImGui::ColorConvertFloat4ToU32(ImVec4(.055f + (e.x - .055f) * t, .055f + (e.y - .055f) * t, .067f + (e.z - .067f) * t, 1.f));
        edge = esp;
    }
    auto Poly = [&](std::initializer_list<ImVec2> pts) {
        ImVec2 v[8];
        int n = 0;
        for (auto& q : pts) v[n++] = P(q.x, q.y);
        d->AddConvexPolyFilled(v, n, fill);
        d->AddPolyline(v, n, edge, ImDrawFlags_Closed, 1.f);
    };
    auto Limb = [&](ImVec2 p0, ImVec2 p1, float w0, float w1) {
        ImVec2 dir = p1 - p0;
        float l = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        ImVec2 n(-dir.y / l, dir.x / l);
        Poly({p0 + n * w0, p0 - n * w0, p1 - n * w1, p1 + n * w1});
    };
    for (int s = 0; s < 2; s++) {
        auto m = [&](float x, float y) { return ImVec2(s ? 160 - x : x, y); };
        Limb(m(52, 72), m(38, 112), 8, 6.5f);
        Limb(m(38, 112), m(29, 158), 6, 4.5f);
        Limb(m(29, 158), m(25, 175), 4.5f, 3);
        Limb(m(67, 154), m(66, 204), 11, 8);
        Limb(m(66, 204), m(68, 262), 7.5f, 4.5f);
        Poly({m(64, 262), m(73, 262), m(77, 278), m(57, 278)});
        for (ImVec2 j : {m(52, 72), m(38, 112), m(66, 204)}) {
            d->AddCircleFilled(P(j.x, j.y), 5 * k, fill);
            d->AddCircle(P(j.x, j.y), 5 * k, edge);
        }
    }
    Limb(ImVec2(80, 52), ImVec2(80, 62), 5.5f, 6);
    Poly({ImVec2(52, 68), ImVec2(108, 68), ImVec2(102, 104), ImVec2(58, 104)});
    Poly({ImVec2(58, 104), ImVec2(102, 104), ImVec2(98, 130), ImVec2(62, 130)});
    Poly({ImVec2(62, 130), ImVec2(98, 130), ImVec2(102, 152), ImVec2(80, 166), ImVec2(58, 152)});
    ImVec2 hv[20];
    for (int i = 0; i < 20; i++) {
        float an = 6.2831853f * i / 20;
        hv[i] = P(80 + 12.5f * std::cos(an), 36 + 15.5f * std::sin(an));
    }
    d->AddConvexPolyFilled(hv, 20, fill);
    d->AddPolyline(hv, 20, edge, ImDrawFlags_Closed, 1.f);

    ImFont* f = ImGui::GetFont();
    float fs = cfg.textSz * k * 1.25f, th = cfg.lineTh;
    auto Txt = [&](const char* s, ImVec2 p, ImU32 c) {
        ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, s), q(p.x - ts.x * .5f, p.y - ts.y * .5f);
        d->AddText(f, fs, q + ImVec2(1, 1), C(0, 0, 0), s);
        d->AddText(f, fs, q, c, s);
    };
    if (cfg.esp) {
        if (cfg.box) {
            ImVec2 b0 = P(20, 12), b1 = P(140, 282);
            if (cfg.boxStyle == 0) d->AddRect(b0, b1, esp, 0, 0, th);
            else {
                float l = 22 * k;
                d->AddLine(b0, ImVec2(b0.x + l, b0.y), esp, th); d->AddLine(b0, ImVec2(b0.x, b0.y + l), esp, th);
                d->AddLine(ImVec2(b1.x, b0.y), ImVec2(b1.x - l, b0.y), esp, th); d->AddLine(ImVec2(b1.x, b0.y), ImVec2(b1.x, b0.y + l), esp, th);
                d->AddLine(ImVec2(b0.x, b1.y), ImVec2(b0.x + l, b1.y), esp, th); d->AddLine(ImVec2(b0.x, b1.y), ImVec2(b0.x, b1.y - l), esp, th);
                d->AddLine(b1, ImVec2(b1.x - l, b1.y), esp, th); d->AddLine(b1, ImVec2(b1.x, b1.y - l), esp, th);
            }
        }
        if (cfg.hp) {
            d->AddRectFilled(P(9, 12), P(14, 282), C(0, 0, 0));
            d->AddRectFilled(P(10, 96), P(13, 281), OK);
        }
        if (cfg.name) Txt("Player_77", P(80, 4), esp);
        if (cfg.dist) Txt("84m", P(80, 298), TX);
        if (cfg.weapon) Txt("AK-47", P(80, 312), T2);
        if (cfg.skel) {
            static const float SK[][4] = {{80, 52, 80, 150}, {52, 70, 80, 58}, {80, 58, 108, 70}, {52, 70, 36, 112}, {36, 112, 29, 158}, {108, 70, 124, 112}, {124, 112, 131, 158},
                                          {66, 152, 80, 150}, {80, 150, 94, 152}, {66, 152, 68, 204}, {68, 204, 68, 266}, {94, 152, 92, 204}, {92, 204, 92, 266}};
            for (auto& s : SK) d->AddLine(P(s[0], s[1]), P(s[2], s[3]), WH, 1.f);
        }
    }
    if (cfg.loot) {
        ImU32 lc = C(226, 183, 43);
        d->AddQuad(P(60, 333), P(65, 328), P(70, 333), P(65, 338), lc, 1.2f);
        ImVec2 ts = f->CalcTextSizeA(fs, FLT_MAX, 0, "Rifle 31m");
        d->AddText(f, fs, ImVec2(P(76, 337).x, P(76, 337).y - ts.y * .5f), lc, "Rifle 31m");
    }
}

inline void Icon(ImDrawList* d, int id, ImVec2 c, float r, ImU32 col) {
    float t = px(1.4f);
    switch (id) {
    case 0:
        d->AddTriangle(ImVec2(c.x - r, c.y + r * .8f), ImVec2(c.x, c.y - r * .9f), ImVec2(c.x + r, c.y + r * .8f), col, t);
        d->AddLine(ImVec2(c.x - r * .4f, c.y + r * .2f), ImVec2(c.x + r * .4f, c.y + r * .2f), col, t);
        break;
    case 1:
        d->AddCircle(c, r * .6f, col, 0, t);
        d->AddLine(ImVec2(c.x, c.y - r), ImVec2(c.x, c.y - r * .3f), col, t); d->AddLine(ImVec2(c.x, c.y + r * .3f), ImVec2(c.x, c.y + r), col, t);
        d->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x - r * .3f, c.y), col, t); d->AddLine(ImVec2(c.x + r * .3f, c.y), ImVec2(c.x + r, c.y), col, t);
        break;
    case 2:
        d->AddBezierCubic(ImVec2(c.x - r, c.y), ImVec2(c.x - r * .5f, c.y - r * .9f), ImVec2(c.x + r * .5f, c.y - r * .9f), ImVec2(c.x + r, c.y), col, t);
        d->AddBezierCubic(ImVec2(c.x - r, c.y), ImVec2(c.x - r * .5f, c.y + r * .9f), ImVec2(c.x + r * .5f, c.y + r * .9f), ImVec2(c.x + r, c.y), col, t);
        d->AddCircle(c, r * .3f, col, 0, t);
        break;
    case 3:
        d->AddCircle(c, r, col, 0, t);
        d->AddLine(ImVec2(c.x - r, c.y), ImVec2(c.x + r, c.y), col, t); d->AddLine(ImVec2(c.x, c.y - r), ImVec2(c.x, c.y + r), col, t);
        break;
    default:
        d->AddLine(ImVec2(c.x - r, c.y - r * .5f), ImVec2(c.x + r, c.y - r * .5f), col, t); d->AddLine(ImVec2(c.x - r, c.y + r * .5f), ImVec2(c.x + r, c.y + r * .5f), col, t);
        d->AddCircleFilled(ImVec2(c.x + r * .3f, c.y - r * .5f), r * .28f, col); d->AddCircleFilled(ImVec2(c.x - r * .3f, c.y + r * .5f), r * .28f, col);
    }
}

inline void Hud() {
    ImGuiIO& io = ImGui::GetIO();
    float s = cfg.hudScale / 100.f;
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 12, 12), ImGuiCond_FirstUseEver, ImVec2(1, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(9 * s, 6 * s));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, C(10, 10, 12, 230));
    ImGui::PushStyleColor(ImGuiCol_Border, LN2);
    ImGui::Begin("##hud", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar |
                                       ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);
    ImGui::SetWindowFontScale(s * .8f);
    ImDrawList* d = ImGui::GetWindowDrawList();
    ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize();
    d->AddRectFilled(wp, ImVec2(wp.x + ws.x, wp.y + 1), Acc());
    const char* st = injected ? "INJECTED" : "WAITING";
    ImU32 sc = injected ? OK : ER;
    char a[16], b[16], tm[8];
    std::snprintf(a, sizeof a, "%d", (int)(io.Framerate + .5f));
    std::snprintf(b, sizeof b, "%d", (int)ping);
    std::time_t tt = std::time(nullptr);
    std::strftime(tm, sizeof tm, "%H:%M", std::localtime(&tt));
    float fs = ImGui::GetFontSize(), mfs = fs * .95f, cs = fs * 1.15f;
    std::string full = std::string("FPS ") + a + "   PING " + b + "   " + tm;
    float w2 = fM->CalcTextSizeA(mfs, FLT_MAX, 0, full.c_str()).x;
    float wc = fB->CalcTextSizeA(cs, FLT_MAX, 0, "CHAD").x, wst = fM->CalcTextSizeA(mfs * .9f, FLT_MAX, 0, st).x + 7 * s;
    float w = std::max(w2, wc + 14 * s + wst), h = cs + 4 * s + mfs;
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::Dummy(ImVec2(w, h));
    d->AddText(fB, cs, p, TX, "CHAD");
    float sx = p.x + w - wst;
    d->AddRectFilled(ImVec2(sx, p.y + cs * .4f), ImVec2(sx + 4 * s, p.y + cs * .4f + 4 * s), sc);
    d->AddText(fM, mfs * .9f, ImVec2(sx + 7 * s, p.y + cs * .12f), sc, st);
    float x = p.x, y = p.y + cs + 4 * s;
    auto T = [&](const char* t, ImU32 c) { d->AddText(fM, mfs, ImVec2(x, y), c, t); x += fM->CalcTextSizeA(mfs, FLT_MAX, 0, t).x; };
    T("FPS ", T3); T(a, TX); T("   PING ", T3); T(b, TX); T("   ", T3); T(tm, TX);
    if (ImGui::IsWindowHovered() && ImGui::IsMouseReleased(0) && io.MouseDragMaxDistanceSqr[0] < 16.f) { cfg.menuOpen = !cfg.menuOpen; Snd(cfg.menuOpen ? 1800.f : 1100.f); }
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);
}

inline void Menu() {
    if (!cfg.menuOpen) return;
    ImGuiIO& io = ImGui::GetIO();
    float W = std::min(px(760), io.DisplaySize.x - 16), H = std::min(px(500), io.DisplaySize.y - 16);
    float sbW = px(140), hdr = px(52), ft = px(22);
    ImGui::SetNextWindowPos(ImVec2((io.DisplaySize.x - W) * .5f, std::min(px(76), io.DisplaySize.y * .15f)), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(W, H));
    ImGui::SetNextWindowBgAlpha(cfg.menuOpacity / 100.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, C(12, 12, 15));
    ImGui::PushStyleColor(ImGuiCol_Border, LN2);
    ImGui::Begin("##chad", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                                        ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse);
    ImGui::SetWindowFontScale(Sc());
    ImDrawList* d = ImGui::GetWindowDrawList();
    ImVec2 wp = ImGui::GetWindowPos();
    static int tab = 0, aimSub = 0, visSub = 0;
    static const char* TN[] = {"Farm", "Aimbot", "Visuals", "World", "Settings"};

    d->AddRectFilled(wp, ImVec2(wp.x + sbW, wp.y + H), C(7, 7, 9, 140));
    d->AddLine(ImVec2(wp.x + sbW, wp.y), ImVec2(wp.x + sbW, wp.y + H), LN);
    d->AddLine(ImVec2(wp.x, wp.y + hdr), ImVec2(wp.x + W, wp.y + hdr), LN);
    d->AddLine(ImVec2(wp.x + sbW, wp.y + H - ft), ImVec2(wp.x + W, wp.y + H - ft), LN);
    ImU32 z = Acc() & 0x00FFFFFFu;
    d->AddRectFilledMultiColor(wp, ImVec2(wp.x + W, wp.y + 1), Acc(), z, z, Acc());

    float m = px(26), lx = wp.x + px(14), ly = wp.y + (hdr - m) * .5f, u = m / 26.f;
    auto R = [&](float x0, float y0, float x1, float y1, ImU32 c) { d->AddRectFilled(ImVec2(lx + x0 * u, ly + y0 * u), ImVec2(lx + x1 * u, ly + y1 * u), c); };
    R(2, 2, 24, 8, C(242, 242, 242)); R(2, 8, 9, 18, C(242, 242, 242)); R(2, 18, 24, 24, C(242, 242, 242)); R(15, 10, 24, 16, Acc());
    d->AddText(fB, px(19), ImVec2(lx + m + px(10), ly - px(2)), C(242, 242, 242), "CHAD");
    Spaced(d, fR, px(8), ImVec2(lx + m + px(10), ly + px(21)), Acc(), "PREMIUM", px(2.4f));

    static const struct { const char* n; int a, b; } SEC[] = {{"Main", 0, 2}, {"Render", 2, 4}, {"Config", 4, 5}};
    float y = wp.y + hdr + px(6);
    for (auto& sec : SEC) {
        d->AddText(ImVec2(wp.x + px(14), y + px(10)), T3, L(sec.n));
        y += px(30);
        for (int i = sec.a; i < sec.b; i++) {
            ImGui::SetCursorScreenPos(ImVec2(wp.x, y));
            ImGui::PushID(i);
            if (ImGui::InvisibleButton("##t", ImVec2(sbW, px(32))) && tab != i) { tab = i; search[0] = 0; Snd(1500.f); }
            ImGui::PopID();
            bool on = tab == i, hv = ImGui::IsItemHovered();
            ImVec2 a(wp.x, y), b(wp.x + sbW, y + px(32));
            if (on) {
                d->AddRectFilledMultiColor(a, b, C(27, 27, 32), C(27, 27, 32, 0), C(27, 27, 32, 0), C(27, 27, 32));
                d->AddRectFilled(a, ImVec2(a.x + 2, b.y), Acc());
            }
            Icon(d, i, ImVec2(a.x + px(21), y + px(16)), px(7.5f), on ? Acc() : (hv ? TX : T2));
            d->AddText(ImVec2(a.x + px(38), y + (px(32) - ImGui::GetFontSize()) * .5f), on ? WH : (hv ? TX : T2), L(TN[i]));
            y += px(32);
        }
    }
    float uy = wp.y + H - px(46);
    d->AddLine(ImVec2(wp.x, uy), ImVec2(wp.x + sbW, uy), LN);
    d->AddRect(ImVec2(wp.x + px(14), uy + px(11)), ImVec2(wp.x + px(38), uy + px(35)), LN2);
    d->AddText(fM, px(11), ImVec2(wp.x + px(22), uy + px(17)), Acc(), "C");
    d->AddText(ImVec2(wp.x + px(46), uy + px(10)), TX, "chad_01");
    d->AddText(fM, px(10), ImVec2(wp.x + px(46), uy + px(26)), T3, "Lifetime");

    float tx = wp.x + sbW + px(16);
    d->AddText(fR, px(14), ImVec2(tx, wp.y + (hdr - px(14)) * .5f), WH, L(TN[tab]));
    float cx = tx + fR->CalcTextSizeA(px(14), FLT_MAX, 0, L(TN[tab])).x + px(16);
    static const char *aimN[] = {"Aimbot", "Silent Aim"}, *visN[] = {"Players", "Chams", "Resources"};
    int ns = tab == 1 ? 2 : tab == 2 ? 3 : 0;
    int* cs = tab == 1 ? &aimSub : &visSub;
    const char** sn = tab == 1 ? aimN : visN;
    for (int i = 0; i < ns; i++) {
        const char* s = L(sn[i]);
        ImVec2 ts = ImGui::CalcTextSize(s);
        float bw = ts.x + px(34), bh = px(26);
        ImGui::SetCursorScreenPos(ImVec2(cx, wp.y + (hdr - bh) * .5f));
        ImGui::PushID(100 + i);
        if (ImGui::InvisibleButton("##s", ImVec2(bw, bh)) && *cs != i) { *cs = i; Snd(1500.f); }
        ImGui::PopID();
        ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax();
        bool on = *cs == i;
        d->AddRectFilled(a, b, on ? C(24, 24, 28) : C(13, 13, 16));
        d->AddRect(a, b, on ? LN2 : LN);
        if (on) d->AddRectFilled(ImVec2(a.x, b.y - 1), b, Acc());
        bool en = tab == 1 ? (i == 0 ? cfg.aim : cfg.silent) : (i == 0 ? cfg.esp : i == 1 ? cfg.chams : cfg.loot);
        d->AddRectFilled(ImVec2(a.x + px(10), a.y + (bh - px(5)) * .5f), ImVec2(a.x + px(15), a.y + (bh + px(5)) * .5f), en ? OK : C(52, 52, 60));
        d->AddText(ImVec2(a.x + px(22), a.y + (bh - ts.y) * .5f), on ? WH : T2, s);
        cx += bw + px(4);
    }

    float cw = px(22), qw = px(140);
    ImGui::SetCursorScreenPos(ImVec2(wp.x + W - px(10) - cw, wp.y + (hdr - cw) * .5f));
    if (ImGui::InvisibleButton("##x", ImVec2(cw, cw))) { cfg.menuOpen = false; Snd(1100.f); }
    {
        ImVec2 a = ImGui::GetItemRectMin(), b = ImGui::GetItemRectMax(), c((a.x + b.x) * .5f, (a.y + b.y) * .5f);
        float q = px(4.5f);
        d->AddRect(a, b, ImGui::IsItemHovered() ? ER : LN);
        d->AddLine(c - ImVec2(q, q), c + ImVec2(q, q), T2, px(1.4f));
        d->AddLine(ImVec2(c.x - q, c.y + q), ImVec2(c.x + q, c.y - q), T2, px(1.4f));
    }
    ImGui::SetCursorScreenPos(ImVec2(wp.x + W - px(10) - cw - px(8) - qw, wp.y + (hdr - px(26)) * .5f));
    ImGui::SetNextItemWidth(qw);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(px(8), px(5)));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, C(13, 13, 16));
    ImGui::PushStyleColor(ImGuiCol_Border, LN);
    ImGui::InputTextWithHint("##q", L("Search element"), search, sizeof search);
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(3);

    static bool drag = false;
    if (ImGui::IsMouseClicked(0) && ImGui::IsMouseHoveringRect(wp, ImVec2(wp.x + W, wp.y + hdr), false) && !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive()) drag = true;
    if (!ImGui::IsMouseDown(0)) drag = false;
    if (drag) {
        ImVec2 np = wp + io.MouseDelta;
        np.x = std::clamp(np.x, 0.f, std::max(0.f, io.DisplaySize.x - W));
        np.y = std::clamp(np.y, 0.f, std::max(0.f, io.DisplaySize.y - H));
        ImGui::SetWindowPos(np);
    }

    ImGui::SetCursorScreenPos(ImVec2(wp.x + sbW + 1, wp.y + hdr + 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(px(14), px(14)));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, px(4));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, 0);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, 0);
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, LN2);
    ImGui::BeginChild("##ct", ImVec2(W - sbW - 1, H - hdr - ft - 2), false);
    ImGui::SetWindowFontScale(Sc());
    gClipA = ImGui::GetWindowPos();
    gClipB = gClipA + ImGui::GetWindowSize();
    gColX = ImGui::GetCursorPosX();

    switch (tab) {
    case 0:
        Box("Resources", ImGui::GetContentRegionAvail().x, [&] {
            ImVec2 p = ImGui::GetCursorScreenPos();
            float W2 = ImGui::GetContentRegionAvail().x, g = px(6), rh = px(54), un = (W2 - 2 * g) / 3.5f, c0 = un * 1.5f, c1 = un, x1 = p.x + c0 + g, x2 = x1 + c1 + g;
            Card("Sulfur", &cfg.sul, p, ImVec2(c0, 2 * rh + g));
            Card("Stone", &cfg.stn, ImVec2(x1, p.y), ImVec2(c1, rh));
            Card("Wood", &cfg.wod, ImVec2(x2, p.y), ImVec2(c1, rh));
            Card("Metal", &cfg.met, ImVec2(x1, p.y + rh + g), ImVec2(c1, rh));
            Card("Magic Farm", &cfg.mag, ImVec2(x2, p.y + rh + g), ImVec2(c1, rh));
            Card("Avoid players 60 m", &cfg.avd, ImVec2(p.x, p.y + 2 * (rh + g)), ImVec2(W2, rh));
            ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + 3 * rh + 2 * g));
            ImGui::Dummy(ImVec2(1, px(8)));
        });
        Cols([&](float w) { Box("Parameters", w, [&] { Slide("Farm distance", &cfg.farmDist, 10, 300, " m"); Slide("Scan speed", &cfg.scanSpeed, 1, 10, "x"); }); },
             [&](float w) { Box("Session", w, [&] { Row("Nodes", "128"); Row("Yield / h", "4.2k"); Row("Uptime", "00:14:32"); }); });
        break;
    case 1:
        if (aimSub == 0)
            Cols([&](float w) { Box("General", w, [&] {
                     Check("Enable", &cfg.aim, &cfg.bAim);
                     Slide("FOV", &cfg.fov, 1, 180, "°");
                     Slide("Smoothness", &cfg.smooth, 1, 100, "%");
                     Slide("Distance", &cfg.aimDist, 10, 500, " m");
                 }); },
                 [&](float w) { Box("Target", w, [&] {
                     static const char* bn[] = {"Head", "Neck", "Chest"};
                     Seg("Hitbox", &cfg.bone, bn, 3);
                     Check("Draw FOV", &cfg.drawFov);
                     Check("Visible check", &cfg.visCheck);
                 }); });
        else
            Cols([&](float w) { Box("General", w, [&] {
                     Check("Enable", &cfg.silent, &cfg.bSil);
                     Slide("Hit chance", &cfg.hitChance, 1, 100, "%");
                     Slide("Distance", &cfg.silentDist, 10, 500, " m");
                 }); },
                 [&](float w) { Box("Filters", w, [&] { Check("Visible only", &cfg.silentVis); }); });
        break;
    case 2: {
        auto pv = [&](float w) { Box("Preview", w, [&] { Preview(); }); };
        if (visSub == 0)
            Cols([&](float w) {
                    Box("Player ESP", w, [&] {
                        static const char* bs[] = {"Full", "Corners"};
                        Check("Enable ESP", &cfg.esp);
                        Check("Box", &cfg.box);
                        Seg("Box style", &cfg.boxStyle, bs, 2);
                        Check("Name", &cfg.name);
                        Check("Health bar", &cfg.hp);
                        Check("Distance", &cfg.dist);
                        Check("Weapon", &cfg.weapon);
                        Check("Skeleton", &cfg.skel);
                    });
                    Box("Render", w, [&] {
                        Slide("Max distance", &cfg.espDist, 50, 500, " m");
                        Slide("Line thickness", &cfg.lineTh, 1, 5, " px");
                        Slide("Text size", &cfg.textSz, 8, 20, " px");
                    });
                    Box("Color", w, [&] { Swatch(&cfg.espCol); });
                }, pv);
        else if (visSub == 1)
            Cols([&](float w) {
                    Box("Chams", w, [&] { Check("Enable chams", &cfg.chams); Slide("Opacity", &cfg.chamsOp, 10, 100, "%"); });
                    Box("Color", w, [&] { Swatch(&cfg.espCol); });
                }, pv);
        else
            Cols([&](float w) { Box("Loot", w, [&] { Check("Loot ESP", &cfg.loot); Slide("Loot distance", &cfg.lootDist, 10, 300, " m"); }); }, pv);
        break;
    }
    case 3:
        Cols([&](float w) { Box("Camera", w, [&] { Check("Free Cam", &cfg.freeCam, &cfg.bCam); Slide("Free cam speed", &cfg.freeCamSpeed, 1, 20, "x"); }); },
             [&](float w) { Box("Environment", w, [&] {
                 Check("Bright Night", &cfg.night);
                 Check("No Grass", &cfg.noGrass);
                 Slide("Time of day", &cfg.tod, 0, 24, " h");
             }); });
        break;
    default:
        Cols([&](float w) { Box("Interface", w, [&] {
                 static const char* ln[] = {"English", "Русский"};
                 Check("Sounds", &cfg.sounds);
                 Seg("Language", &cfg.lang, ln, 2);
                 Slide("Menu opacity", &cfg.menuOpacity, 30, 100, "%");
                 Slide("Menu size", &cfg.menuScale, 70, 120, "%");
                 Slide("HUD scale", &cfg.hudScale, 60, 160, "%");
             }); },
             [&](float w) { Box("Accent", w, [&] {
                 static const char* an[] = {"Purple", "Blue", "Red"};
                 Seg("Accent", &cfg.accent, an, 3, ACC, false);
             }); });
    }
    ImGui::EndChild();
    ImGui::PopStyleColor(3);
    ImGui::PopStyleVar(2);

    const char* bt = "build 2.4.1";
    float bs = px(9.5f);
    d->AddText(fM, bs, ImVec2(wp.x + sbW + px(12), wp.y + H - ft + px(5)), T3, "game.exe · pid 8412");
    d->AddText(fM, bs, ImVec2(wp.x + W - px(12) - fM->CalcTextSizeA(bs, FLT_MAX, 0, bt).x, wp.y + H - ft + px(5)), T3, bt);
    ImGui::End();
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar(4);
}

inline void Render() {
    if (!fR) LoadFonts();
    Hud();
    Menu();
}

}  // namespace chad
