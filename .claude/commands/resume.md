Resume from a session checkpoint.

1. List all checkpoint files in `.claude/checkpoints/` sorted by date
   (most recent first). Show filename and first line of each file.

2. Ask the user which checkpoint to load.

3. Once selected, read the checkpoint file and summarize:
   - What the session was about
   - Where it left off
   - What the next steps were

4. Ask the user if they want to continue from there or need
   clarification on the context first.
