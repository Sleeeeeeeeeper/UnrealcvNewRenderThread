# UnrealcvNewRenderThread
Add RecordCommand


1.Add a camera(unrealcv/private/recording/saveRenderCamera)
2.Add a actor tag to camera


run command

 
vset /action/rec/start cameraTag imageType filepath deltaTime width height imageNumber
7 Parameters ,you need type 1 at least[camera tag].

vset /action/rec/stop cameraTag
1 parameter ,to stop record.
