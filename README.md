# 🎮 No Tomorrow (Project Heist)

**No Tomorrow** is a top-down immersive sim set in a collapsing world where the player plans and executes heists while facing anomalies and moral dilemmas.

---

## 🧠 About the Game

> **"If the world no longer gives you chances — go and take yours."**

- **Genre:** Immersive Sim / Tactical Stealth / Top-down  
- **Engine:** Unreal Engine 5.5.4  
- **Platforms:** PC
- **Codebase:** C++ / Blueprints  
- **Features:** Anomalies, deep interactivity, multiple approaches to progression, branching story

---

## 🚀 How to Run

1. **Clone the repository**:
   ```bash
   git clone https://github.com/your-name/no-tomorrow.git
   ```

2. **Open the `.uproject` file with Unreal Engine 5.5.4**

3. If you need to build:
   - Open `NoTomorrow.sln` in Visual Studio
   - Select `Development Editor` configuration, platform `Win64`
   - Click `Build`

4. **Play in Editor (PIE)** — press `Play` in the toolbar

---

## ✅ Requirements

- **Unreal Engine:** 5.5.4
- **Visual Studio:** 2022 or newer (with "Game development with C++" installed)
- **ReSharper C++ (optional)** for improved autocomplete and analysis

---

## 📁 Folders

| Folder | Purpose |
|--------|---------|
| `/Art/`             | Models, materials, textures |
| `/Audio/`           | Sounds and music |
| `/Blueprints/Core/` | GameMode, Controller, Character |
| `/Blueprints/UI/`   | User Interface |
| `/Blueprints/Data/` | DataTables, Enums, Configs |
| `/Maps/`            | Playable maps |

---

## 🧾 File Naming Convention

We use consistent prefixes and conventions to help organize and search assets and code more easily.

### 🔷 Blueprint & Asset Prefixes

| Prefix | Description                  | Example                |
|--------|------------------------------|------------------------|
| `BP_`  | Blueprint                    | `BP_PlayerCharacter`   |
| `W_` | Widget Blueprint (UI)        | `WBP_MainMenu`         |
| `DT_`  | Data Table                   | `DT_WeaponStats`       |
| `DA_`  | Data Asset                   | `DA_AnomalyType`       |
| `T_`   | Texture                      | `T_Icon_Health`        |
| `M_`   | Material                     | `M_AnomalyDistortion`  |
| `MI_`  | Material Instance            | `MI_AnomalyBlue`       |
| `SM_`  | Static Mesh                  | `SM_Locker_01`         |
| `SK_`  | Skeletal Mesh                | `SK_Character_Thief`   |
| `SFX_` | Sound Effect                 | `SFX_Gunshot`          |

### 🟦 C++ Naming Convention

| Element       | Convention                                                 | Example                      |
|---------------|------------------------------------------------------------|------------------------------|
| Class         | Unreal-style prefix (`A` or `U`) + `NT` + PascalCase       | `class ANTInventoryManager` / `class UNTInventorySubsystem`   |
| Interface     | `INT` prefix + PascalCase                                  | `class INTInteractable`      |
| Struct        | `F` prefix + PascalCase                                    | `struct FAnomalyData`        |
| Enum          | `E` prefix + PascalCase                                    | `enum class EAnomalyType`    |
| Variable      | CamelCase                                                  | `playerHealth`, `isVisible`  |
| Function      | PascalCase                                                 | `BeginPlay()`, `Interact()`  |
| UPROPERTY     | `UPROPERTY(...)` + clear name                              | `UPROPERTY(EditAnywhere) float DetectionRange;` |
| UFUNCTION     | `UFUNCTION(...)` + PascalCase                              | `UFUNCTION(BlueprintCallable) void ResetState();` |

**General Rules:**
- Always use descriptive, context-aware names.
- Avoid abbreviations unless widely understood.
- Keep consistency across the codebase and assets.

---
