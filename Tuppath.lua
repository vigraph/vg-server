function get_dependency_path(name)
  if name == 'ot-xmlmesh-core' then
    return 'obtools/xmlmesh/core'
  elseif name == 'ot-xmlmesh-send' then
    return 'obtools/xmlmesh/bindings/cli/send'
  elseif name == 'ot-xmlmesh-receive' then
    return 'obtools/xmlmesh/bindings/cli/receive'
  elseif string.sub(name, 1, 11) == 'ot-xmlmesh-' then
    return 'obtools/xmlmesh/' .. string.sub(name, 12)
  elseif name == 'ot-toolgen' then
    return 'obtools/tools/toolgen'
  elseif string.sub(name, 1, 11) == 'ot-obcache-' then
    return 'obtools/obcache/libs/' .. string.sub(name, 12)
  elseif string.sub(name, 1, 3) == 'ot-' then
    return 'obtools/libs/' .. string.sub(name, 4)

  elseif name == 'vg-engine' then
    return 'engine'

  elseif string.sub(name, 1, 15) == 'vg-module-core-' then
    return 'modules/core/' .. string.sub(name, 16)
  elseif string.sub(name, 1, 17) == 'vg-module-vector-' then
    return 'modules/vector/' .. string.sub(name, 18)
  elseif string.sub(name, 1, 13) == 'vg-module-ui-' then
    return 'modules/ui/' .. string.sub(name, 14)
  elseif string.sub(name, 1, 16) == 'vg-module-laser-' then
    return 'modules/laser/' .. string.sub(name, 17)
  elseif string.sub(name, 1, 15) == 'vg-module-midi-' then
    return 'modules/midi/' .. string.sub(name, 16)
  elseif string.sub(name, 1, 16) == 'vg-module-audio-' then
    return 'modules/audio/' .. string.sub(name, 17)
  elseif string.sub(name, 1, 14) == 'vg-module-dmx-' then
    return 'modules/dmx/' .. string.sub(name, 15)
  elseif string.sub(name, 1, 17) == 'vg-module-sensor-' then
    return 'modules/sensor/' .. string.sub(name, 18)
  elseif string.sub(name, 1, 22) == 'vg-module-time-series-' then
    return 'modules/time-series/' .. string.sub(name, 23)

  elseif string.sub(name, 1, 3) == 'vg-' then
    return 'libs/' .. string.sub(name, 4)
  end
  return nil
end
