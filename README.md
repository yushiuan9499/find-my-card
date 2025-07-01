# Find My Card

## Installation
```bash
./tools/build.sh
```

## Usage
1. Prepare two json files in a directory:
  - `actions.json`: Contains the actions to be performed.
  - `scenario0.json`: Contains the scenarios to be tested. See the example in `json/`.
  
2. Run the command:
    ```bash
    ./build/main <directory>
    ```
3. The output will be saved in the same directory as `scenario<num>.json`.

## Scenarios
- `normal/`: 描述正常的情形。
- `fakeCard/`: 描述攻擊者偽造卡片的情形。
- `hackNoVeri`: 描述攻擊者沒有取得 Email 驗證碼的情形。
- `hackWVeri`: 描述供給者取得過時的 2PA 驗證碼的情形。
- `fakeBox`: 描述攻擊者偽造盒子的情形。
