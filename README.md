# Cold Turkey Blocker Universal Patcher
Cold Turkey Blocker Universal Patcher allows you to edit the contents of a Cold Turkey database stored in SQLite format.

## Usage

1. Clone this repository or download the release (pre-compiled binary).

2. Launch the patcher (`CTB-DB Patcher.exe`).

3. The application will search for the Cold Turkey database file in the default path `C:\ProgramData\Cold Turkey\data-app.db`. If the file exists, it will proceed to edit the database. If not, it will prompt you to select the database file using a file dialog.

4. The updated database will be saved, and a success message will be displayed.

## Limitations

- The application assumes a specific structure for the Cold Turkey database and the "settings" table. It may not work correctly if the database structure or table structure is different.

- The application overwrites the existing "proStatus" value with "pro" without any further validation or checks.

- The application only modifies the "settings" table and the "proStatus" value. Additional modifications to other tables or values would require further development.
