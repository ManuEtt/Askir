# GitHub import – ManuEtt/Askir

1. Download and extract the ZIP. The files inside `AskirRPG/` are the repository root.
2. On your Windows development PC install Git and Git LFS if you later add large `.uasset`, `.fbx`, or `.glb` files. This archive does **not** require LFS for its small current assets.
3. Clone `https://github.com/ManuEtt/Askir`.
4. Copy the **contents** of the extracted `AskirRPG` directory into the cloned repository root. Do not copy the enclosing folder unless you deliberately want a nested Unreal project.
5. Review the changes with `git status` and verify that no `Binaries/`, `Intermediate/`, or `Saved/` files are staged.
6. Run `python Tools/validate_release.py` and `python -m unittest discover -s Tests -v`.
7. `git add . && git commit -m "Askir: 50-step development batch V20" && git push origin main` (replace `main` if your default branch differs).
8. Double-click `AskirRPG.uproject` with Unreal Engine 5.6; generate Visual Studio files and compile. This cannot be validated by the repository-only CI.
9. The GitHub workflow runs Python static checks; it does **not** compile Unreal C++.
10. Never commit access tokens or third-party assets without distribution rights.

If GitHub already contains files, inspect conflicts before copying or pushing. This ZIP is not itself a Git commit or an automatic push.
