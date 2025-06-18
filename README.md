# Find My Card
### [PPT](https://www.canva.com/design/DAGqsJHxZ78/RSLW6EoJUfBwLmd88Ha6_Q/edit)

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

## Actions
==Documentation Not Done Yet==
The actions that can be performed are defined in `actions.json`. 
### addCard
- action: `addCard`
- description: Adds a card to the server.
