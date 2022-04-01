; create scheme variables
(rp-var-define 'udf/convergence 1 'int #f)
(rp-var-define 'udf/ongoing 1 'int #f)
(rp-var-define 'udf/iterate 0 'int #f)
(rp-var-define 'udf/checkpoint 0 'int #f)
(rp-var-define 'udf/config-location "./precice-config.xml" 'string #f)
(rp-var-define 'solve/dt 0.5 'real #f)

; assign values to variables created
(rpsetvar 'udf/convergence 1)
(rpsetvar 'udf/ongoing 1)
(rpsetvar 'udf/iterate 0)
(rpsetvar 'udf/checkpoint 0)

; make sure case is using fixed time stepping method
(rpsetvar 'time/adaptive/dt-method 0)

;;; USER CAN EDIT THESE ;;;
(rpsetvar 'udf/config-location "./precice-config.xml")
(rpsetvar 'solve/dt 1.0)
;;;;;;;;;;;;;;;;;;;;;;;;;;;

; initialize flow field
(ti-menu-load-string (string-append "solve/set/time-step " (number->string(%rpgetvar 'solve/dt))))
(rpsetvar 'dynamesh/update-in-timestep/residual-criterion -1.0)
(ti-menu-load-string "solve/initialize/initialize-flow")
