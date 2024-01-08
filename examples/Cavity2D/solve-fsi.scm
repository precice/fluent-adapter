; tell Fluent to never really update the mesh, for now -- 1000000 is the max update interval allowed
(rpsetvar 'dynamesh/update-in-timestep/update-interval 1000000)

(do () ((= (%rpgetvar 'udf/ongoing) 0))
   ; preCICE was initilized, which could have modified the time step, so we
   ; reset the time step here
   (ti-menu-load-string (string-append "solve/set/time-step " (number->string(%rpgetvar 'solve/dt))))
   (display "\n")
   ; advance the solver to the next time step
   (ti-menu-load-string "solve/dual-time-iterate 1 0")
   (if (= (%rpgetvar 'udf/iterate) 0)
     ; if we're not solving a time step that ends in preCICE coupling
     ; (sub-cycling), then just run the fluid solver a certain number of
     ; iterations
     ( begin 
        (ti-menu-load-string "solve/iterate 101")
     )
     ; if we are coupling with preCICE, start a loop that checks for coupling
     ; convergence
     ( begin
        (do () ((= (%rpgetvar 'udf/convergence) 1))
           ; run the fluid solver a certain number of iterations
           (ti-menu-load-string "solve/iterate 32")
           ; write forces to preCICE and advance
           (ti-menu-load-string "define/user-defined/execute-on-demand \"write_and_advance::libudf\"")
           ; if preCICE says we are not converged yet, update the fluid grid with
           ; deformations from the solid solver
           (if (= (%rpgetvar 'udf/convergence) 0)
           ( begin 
              (rpsetvar 'dynamesh/update-in-timestep/update-interval 1)
              (ti-menu-load-string "solve/iterate 1")
              (rpsetvar 'dynamesh/update-in-timestep/update-interval 1000000)
           )
           )
        )
     )
   )
   ; write a checkpoint, if needed
   (if (= (%rpgetvar 'udf/checkpoint) 1)
   ( begin
      (ti-menu-load-string "file/write-case-data fluent_checkpoint.cas yes")
   ))
)
