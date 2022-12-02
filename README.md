# They_are_back_again
 
Developed with Unreal Engine 5.1

Requires advanced input to be able to play

Working basic functions,
- needs more commenting
- needs some refactoring
- has to unify communication of preference between some files
- loading soft object references stored in data assets make some trouble on loading, but in that case a (not nice) code will handle it, and force load them.
Data asset loads fine, it can even get the path of the asset from tha SO reference, but on validity check it fails, so can't call Async load on them.
- some quite rare unhandled crash happened, but couldn't replicate, probably some Editor/Livecoding issue


package: https://drive.google.com/drive/folders/1Y5UZJ7mVH-ohUHXAC8Crtpi8g6R2wSQm