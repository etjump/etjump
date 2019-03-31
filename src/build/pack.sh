echo "creating etjump pk3..."
CWD_DIR="$PWD"
cd ../../etjump
zip -r "$CWD_DIR/unversioned_etjump.pk3" .
cd "$CWD_DIR"
zip unversioned_etjump.pk3 cgame.* ui.*
echo "done"

