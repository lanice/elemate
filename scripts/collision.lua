
function particleBboxCollision(group1id, group2id, llf_x, llf_y, llf_z, urb_x, urb_y, urb_z)
    io.write("Collision detected: ", group1id, ":", psa_elementAtId(group1id), " - ", group2id, ":", psa_elementAtId(group2id), "\n")
    io.write(llf_x, ":", llf_y, ":", llf_z, " - ", urb_x, ":", urb_y, ":", urb_z, "\n")
end
